#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

const int bubble_radius = 40;

typedef struct bubble {
    Vector2 position, velocity;
    int radius;
    Color color;
} Bubble;

typedef struct bubble_node {
    Bubble bubble;
    struct bubble_node *next;
} BubbleNode;

typedef struct {
    BubbleNode *head, *tail;
} BubbleLinkedList;

float randomVelocity(int min, int max, int precision) {
    return ((float)GetRandomValue(min * precision, max * precision)) / precision;
}

Color randomColor(float saturation, float value) {
    float hue = (float)GetRandomValue(0, 360);
    return ColorFromHSV(hue, saturation, value);
}

Bubble randomBubble(int radius) {
    Bubble bubble = {
        .position.x = (float)GetRandomValue(radius, GetScreenWidth() - radius),
        .position.y = (float)GetRandomValue(radius, GetScreenHeight() - radius),

        .radius = radius,

        .velocity.x = randomVelocity(-10, 10, 2),
        .velocity.y = randomVelocity(-10, 10, 2),

        .color = randomColor(0.6, 0.7),
    };

    return bubble;
}

void appendNode(BubbleLinkedList *list, BubbleNode *node) {
    if (!list->head) {
        list->head = node;
    }
    if (list->tail) {
        list->tail->next = node;
    }
    list->tail = node;
}

BubbleLinkedList initBubbles(int count) {
    BubbleLinkedList list = { .head = NULL, .tail = NULL };

    for (int i = 0; i < count; i++) {
        BubbleNode *node = malloc(sizeof(BubbleNode));
        node->next = NULL;
        node->bubble = randomBubble(bubble_radius);

        appendNode(&list, node);
    }

    return list;
}

void destroyBubbleLinkedList(BubbleLinkedList bubble_list) {
    if (!bubble_list.head) return;

    BubbleNode *p_next;

    for (BubbleNode *p = bubble_list.head; p; p = p_next) {
        p_next = p->next;
        free(p);
    }
}

void updateBubblePosition(BubbleLinkedList bubble_list) {
    for (BubbleNode *p = bubble_list.head; p; p = p->next) {
        p->bubble.position.x += p->bubble.velocity.x;
        p->bubble.position.y += p->bubble.velocity.y;
    }
}

void resolveCollision(BubbleLinkedList bubble_list) {
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();

    // Border collision
    for (BubbleNode *p = bubble_list.head; p; p = p->next) {
        // Border left
        if (p->bubble.position.x < p->bubble.radius) {
            p->bubble.position.x = p->bubble.radius;
            p->bubble.velocity.x = -p->bubble.velocity.x;
        }
        // Border bottom
        if (p->bubble.position.y > screen_height - p->bubble.radius) {
            p->bubble.position.y = screen_height - p->bubble.radius;
            p->bubble.velocity.y = -p->bubble.velocity.y;
        }
        // Border right
        if (p->bubble.position.x > screen_width - p->bubble.radius) {
            p->bubble.position.x = screen_width - p->bubble.radius;
            p->bubble.velocity.x = -p->bubble.velocity.x;
        }
        // Border up
        if (p->bubble.position.y < p->bubble.radius) {
            p->bubble.position.y = p->bubble.radius;
            p->bubble.velocity.y = -p->bubble.velocity.y;
        }
    }

    // Bubble collision
    for (BubbleNode *bubble_a = bubble_list.head; bubble_a; bubble_a = bubble_a->next) {
        for (BubbleNode *bubble_b = bubble_list.head; bubble_b; bubble_b = bubble_b->next) {
            if (bubble_a == bubble_b) {
                // Same bubble
                continue;
            }

            Bubble a = bubble_a->bubble, b = bubble_b->bubble;

            if (CheckCollisionCircles(a.position, a.radius, b.position, b.radius)) {
                Vector2 normal = Vector2Normalize(Vector2Subtract(b.position, a.position));
                Vector2 relativeVelocity = Vector2Subtract(b.velocity, a.velocity);

                float normalVelocity = Vector2DotProduct(relativeVelocity, normal);
                if (normalVelocity > 0) {
                    // Separating
                    continue;
                }

                Vector2 impulse = {
                    .x = normal.x * -normalVelocity,
                    .y = normal.y * -normalVelocity,
                };

                bubble_a->bubble.velocity = Vector2Subtract(a.velocity, impulse);
                bubble_b->bubble.velocity = Vector2Add(b.velocity, impulse);
            }
        }
    }
}

void handleMouseDown(BubbleLinkedList *bubble_list) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return;

    Vector2 mouse_position = GetMousePosition();
    for (BubbleNode *p = bubble_list->head; p; p = p->next) {
        if (CheckCollisionCircles(mouse_position, bubble_radius, p->bubble.position, p->bubble.radius)) {
            return;
        }
    }

    // Create new bubble
    BubbleNode *node = malloc(sizeof(BubbleNode));
    node->next = NULL;
    node->bubble.position = mouse_position;
    node->bubble.radius = bubble_radius;

    node->bubble.velocity.x = randomVelocity(-10, 10, 2);
    node->bubble.velocity.y = randomVelocity(-10, 10, 2);

    node->bubble.color = randomColor(0.6, 0.7);

    appendNode(bubble_list, node);
}

int main() {
    const int init_width = 1024;
    const int init_height = 768;

    const int count = 10;

    InitWindow(init_width, init_height, "Bubbles");
    SetTargetFPS(60);

    SetWindowState(FLAG_FULLSCREEN_MODE | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT);

    BubbleLinkedList bubble_list = initBubbles(count);

    while (!WindowShouldClose()) {
        ClearBackground(BLANK);

        handleMouseDown(&bubble_list);

        BeginDrawing();

        for (BubbleNode *p = bubble_list.head; p; p = p->next) {
            DrawCircleV(p->bubble.position, p->bubble.radius, p->bubble.color);
        }

        EndDrawing();

        updateBubblePosition(bubble_list);
        resolveCollision(bubble_list);
    }

    CloseWindow();
    destroyBubbleLinkedList(bubble_list);
    return 0;
}
