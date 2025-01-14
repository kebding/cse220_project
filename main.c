#include "AStar.h"
#include <stdio.h>

#define MAX_NODES   1024
#define MAX_POS     400
#define HOP_LEN     4
#define X_STEP      3
#define Y_STEP      3
#define Y_PER_ROW   120

typedef struct node {
    int x;
    int y;
    struct node* neighbors[MAX_NODES];
} node;

static float hopCost(void *srcNode, void *dstNode, void *context) {
    node* src = (node*)srcNode;
    node* dst = (node*)dstNode;

    /* using the Manhattan distance because the sqrt() and pow() functions
       needed for Euclidean distance are part of math.h, which is an external
       library that I would have to rebuild as a RISC-V library
    */
    float x_delta, y_delta;
    // to avoid needing abs(), subtract the smaller from the greater
    x_delta = (src->x > dst->x) ? (src->x - dst->x) : (dst->x - src->x);
    y_delta = (src->y > dst->y) ? (src->y - dst->y) : (dst->y - src->y);
    return x_delta + y_delta;
}

static void nodeNeighbors(ASNeighborList neighbors, void* srcNode, void* context) {
    node* src = (node*)srcNode;
    int i;
    for (i = 0; i < MAX_NODES; i++) {
        if (src->neighbors[i]) {
            ASNeighborListAdd(neighbors, (void*)src->neighbors[i], 
                    hopCost(srcNode, (void*)(src->neighbors[i]), (void*)NULL));
        }
    }
}

static const ASPathNodeSource pathSource = {
    sizeof(node), 
    &nodeNeighbors,
    &hopCost,
    NULL,
    NULL
};

int main(int argc, char** argv) {
    node* graph[MAX_NODES];
    int i, j;
    for (i = 0; i < MAX_NODES; i++) {
        graph[i] = calloc(1, sizeof(node));
        graph[i]->x = i*X_STEP % MAX_POS;
        graph[i]->y = (int)(i/Y_PER_ROW)*Y_STEP % MAX_POS;
    }
    for (i = 0; i < MAX_NODES; i++) {
        //printf("node = %d. x = %d, y = %d\n", i, graph[i]->x, graph[i]->y);
    }
    int hopCount;
    float cost;
    ASPath path;
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            if (hopCost(graph[i], graph[j], NULL) < HOP_LEN) {
                graph[i]->neighbors[j] = graph[j];
                graph[j]->neighbors[i] = graph[i];
            }
        }
    }
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            path = ASPathCreate(&pathSource, (void*)NULL, graph[i], graph[j]);
            cost = ASPathGetCost(path);
            hopCount = ASPathGetCount(path);
            //printf("path from %d to %d: cost=%f, hopCount=%d\n", i, j, cost, hopCount);
            ASPathDestroy(path);
        }
    }
    return 0;
}
