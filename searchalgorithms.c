#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ROWS 4
#define COLS 5

bool debug = false;

// Define constants for actions
#define SUCK 0
#define DOWN 1
#define UP 2
#define RIGHT 3
#define LEFT 4

#define BOOST 0.0





// Define data structures
typedef struct {
    int x;  // x-coordinate
    int y;  // y-coordinate
} Location;

typedef struct {
    bool grid[COLS][ROWS];
    Location agent;
} State;

typedef struct Node {
    State state;
    int action;
    struct Node* parent;
    int depth;
    double pathCost;
} Node;

Node* initializeWorld(int instance){
    State initialState;
    
    // Initialize the grid to all clean rooms (0)
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            initialState.grid[i][j] = true;
        }
    }

    // Set the agent's starting location based on the problem instance
    
    if (instance == 1) {
        initialState.agent.x = 2;
        initialState.agent.y = 2;
        initialState.grid[0][1] = false; // Room (1, 2) is dirty
        initialState.grid[1][3] = false; // Room (2, 4) is dirty
        initialState.grid[2][4] = false; // Room (3, 5) is dirty
    } else if (instance == 2) {
        initialState.agent.x = 2;
        initialState.agent.y = 3;
        initialState.grid[0][1] = false; // Room (1, 2) is dirty
        initialState.grid[1][0] = false; // Room (2, 1) is dirty
        initialState.grid[1][3] = false; // Room (2, 4) is dirty
        initialState.grid[2][2] = false; // Room (3, 3) is dirty
    }
    else if (instance == 3) { //testing case
        initialState.agent.x = 2;
        initialState.agent.y = 2;
        initialState.grid[1][2] = false; // Room (1, 1) is dirty
        initialState.grid[1][0] = false; // Room (1, 1) is dirty
    }
    
    Node* rootNode = (Node*)malloc(sizeof(Node));;
    rootNode->state = initialState;
    rootNode->parent = NULL;
    rootNode->depth = 0;
    rootNode->pathCost = 0.0;
    return rootNode;
}

bool isGoalState(State state){
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (state.grid[i][j] == 0) {
                // If any room is dirty, the goal state is not reached
                return false;
            }
        }
    }

    // All rooms are clean, so the goal state is reached
    return true;
}

bool nodesEqual(Node* n1, Node* n2){
    if (n1->state.agent.x != n2->state.agent.x || n1->state.agent.y != n2->state.agent.y){return false;} //differing agents
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            if (n1->state.grid[i][j] != n2->state.grid[i][j]){
               return false;  //they are not the same!
            }
        }
    }
    return true; //they are the same!
}

//(expandNode) Function to check if the agent is capable of the current move
bool isActionApplicable(State state, int action) {
    Location agentLoc = state.agent;

    switch (action) {
        case LEFT:
            // Check if moving left is within bounds
            if (agentLoc.x > 1) {
                return true;
            }
            break;
        case RIGHT:
            // Check if moving right is within bounds
            if (agentLoc.x < COLS) {
                return true;
            }
            break;
        case UP:
            // Check if moving up is within bounds
            if (agentLoc.y > 1) {
                return true;
            }
            break;
        case DOWN:
            // Check if moving down is within bounds
            if (agentLoc.y < ROWS) {
                return true;
            }
            break;
        case SUCK:
            return true; //SUCK action. This is always allowed, even in a clean room.
            break;
    }

    // Action is not applicable in the current state
    return false;
}

//(expandNode) Function to apply an action to a state and generate a new state
State applyAction(State state, int action) {
    State newState = state; // Copy the current state

    // Update the agent's location based on the action
    switch (action) {
        case LEFT:
            newState.agent.x--;
            break;
        case RIGHT:
            newState.agent.x++;
            break;
        case UP:
            newState.agent.y--;
            break;
        case DOWN:
            newState.agent.y++;
            break;
        case SUCK:
            newState.grid[newState.agent.y - 1][newState.agent.x - 1] = true;
            break;
    }

    return newState;
}

// Function to expand a node and generate successor nodes
Node** expandNode(Node* node, int* numSuccessors, bool boost){
    //5: Suck, Down, Up, Right, Left
    Node** successors = (Node**)malloc(5 * sizeof(Node*));
    if (successors == NULL) {
        perror("Memory allocation failed.");
        exit(1);
    }
    *numSuccessors = 0;
    int actions[5] = {SUCK, DOWN, UP, RIGHT, LEFT};

    for (int i = 0; i < 5; i++) {
        // Check if the action is applicable in the current state
        if (isActionApplicable(node->state, actions[i])) {
            // Create a new state by applying the action
            State newState = applyAction(node->state, actions[i]);
            // Create a new node for the successor
            Node* successor = (Node*)malloc(sizeof(Node));
            if (successor == NULL) {
                perror("Memory allocation failed.");
                exit(1);
            }
            
            // Set the successor node's attributes
            successor->state = newState;
            successor->action = actions[i];
            successor->parent = node;
            successor->depth = node->depth + 1;
            
            //determine the step cost:
            double stepCost = 0.0;
            switch (actions[i]) {
                case LEFT:
                    stepCost+=1.0;
                    break;
                case RIGHT:
                    stepCost+=0.9;
                    break;
                case UP:
                    stepCost+=0.8;
                    break;
                case DOWN:
                    stepCost+=0.7;
                    break;
                case SUCK:
                    stepCost+=0.6;
                    if(boost){
                        if ((node->state.grid[node->state.agent.y - 1][node->state.agent.x - 1] == false)&&(newState.grid[newState.agent.y - 1][newState.agent.x - 1] == true)){
                            stepCost-=BOOST;//cleaned up dirt!
                        }
                    }
                    break;
            }
            successor->pathCost = node->pathCost + stepCost;

            // Add the successor to the array of successors
            successors[*numSuccessors] = successor;
            (*numSuccessors)++;
        }
    }

    return successors;
}

//fringe implementation:
///queue for fringe
typedef struct {
    Node** array;
    int capacity;
    int size;
} Fringe;
///creates the queue (with a limit)
Fringe* createFringe(int capacity) {
    Fringe* pq = (Fringe*)malloc(sizeof(Fringe));
    pq->capacity = capacity;
    pq->size = 0;
    pq->array = (Node**)malloc(capacity * sizeof(Node*));
    return pq;
}
///swaps two nodes in the queue
void swap(Node** a, Node** b) {
    Node* temp = *a;
    *a = *b;
    *b = temp;
}
///arranges nodes in queue from smallest to largest
void sortByCost(Fringe* fringe) {
    for (int i = 0; i < fringe->size - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < fringe->size; j++) {
            // Compare pathCosts
            if (fringe->array[j]->pathCost < fringe->array[minIndex]->pathCost) {
                minIndex = j;
            }
        }
        // Swap the nodes to place the minimum at the beginning
        swap(&fringe->array[i], &fringe->array[minIndex]);
    }
}
//arranges nodes in queue from lowest to highest depth
void sortByDepth(Fringe* fringe) {
    for (int i = 0; i < fringe->size - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < fringe->size; j++) {
            // Compare depths
            if (fringe->array[j]->depth < fringe->array[minIndex]->depth) {
                minIndex = j;
            }
        }
        // Swap the nodes to place the minimum at the beginning
        swap(&fringe->array[i], &fringe->array[minIndex]);
    }
}

///extracts least costly member of the fringe
//mode true: tie-break for matching pathCost. mode false: tie-break for matching depth.
Node* extractMin(Fringe* q, bool mode) {
    
    if (q->size == 0) {
        return NULL;
    }

    Node* root = q->array[0];

    if (q->size > 1) {
        
        // tie breaker. go through the matching nodes and find the one farthest to the top left.
        if (mode){//tie-break for pathCost
            if (debug) {
                printf("\nBEFORE QUEUE:\n");
                for (int i = 0; i < q->size; i++){
                    printf("%.1f(%d,%d), ", q->array[i]->pathCost, q->array[i]->state.agent.y, q->array[i]->state.agent.x);
                }
            }
            double cost = root->pathCost;
            int i = 1;
            while (i<q->size && q->array[i]->pathCost < cost+0.1){
                if (debug){printf("tryout!\n");}
                if (q->array[i]->state.agent.y < root->state.agent.y || (q->array[i]->state.agent.y == root->state.agent.y && q->array[i]->state.agent.x < root->state.agent.x)){
                    if (debug){printf("swap time!\n");}
                    swap(&(q->array[0]),&(q->array[i]));
                    root = q->array[0];
                }
                i++;
            }
        
            swap(&(q->array[0]),&(q->array[q->size - 1]));
            root = q->array[q->size - 1];
            q->size--; //selected node will now be excluded from the sort
            sortByCost(q);
        }
        else{//tie-break for depth
            if (debug) {
                printf("\nBEFORE QUEUE:\n");
                for (int i = 0; i < q->size; i++){
                    printf("%d(%d,%d), ", q->array[i]->depth, q->array[i]->state.agent.y, q->array[i]->state.agent.x);
                }
            }
            int minDepth = root->depth;
            int i = 1;
            while (i<q->size && q->array[i]->depth == minDepth){
                if (q->array[i]->state.agent.y < root->state.agent.y || (q->array[i]->state.agent.y == root->state.agent.y && q->array[i]->state.agent.x < root->state.agent.x)){
                    swap(&(q->array[0]),&(q->array[i]));
                    root = q->array[0];
                }
                i++;
            }
            swap(&(q->array[0]),&(q->array[q->size - 1]));
            root = q->array[q->size - 1];
            q->size--; //selected node will now be excluded from the sort
            sortByDepth(q);            
        }
        
    }
    else{q->size--;}
    if (debug) {
        printf("\nAFTER QUEUE:\n");
        if (mode){
            for (int i = 0; i < q->size; i++){
                printf("%.1f(%d,%d), ", q->array[i]->pathCost, q->array[i]->state.agent.x, q->array[i]->state.agent.y);
            }
            printf("\nextracted node: %.1f(%d,%d)\n", root->pathCost, root->state.agent.y, root->state.agent.x);
        }
        else{
            for (int i = 0; i < q->size; i++){
                printf("%d(%d,%d), ", q->array[i]->depth, q->array[i]->state.agent.y, q->array[i]->state.agent.x);
            }
            printf("\nextracted node: %d(%d,%d)\n", root->depth, root->state.agent.y, root->state.agent.x);
        }
    }
    
    return root;
}
///inserts a node into the fringe
void insert(Fringe* q, Node* node) {
    if (q->size == q->capacity) {
        printf("queue is full.\n");
        return;
    }

    int i = q->size;
    q->array[i] = node;
    q->size++;
}

void printWorld(Node* node){
    printf("\n");
    char* actions[5] = {"SUCK", "DOWN", "UP", "RIGHT", "LEFT"};
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d", node->state.grid[i][j]);
            if(j+1 == node->state.agent.x && i+1 == node->state.agent.y){
                printf("V");
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("Vacuum is at (%d,%d)\n",node->state.agent.y,node->state.agent.x);
    printf("current pathCost: %.1f, depth: %d, action: %s\n", node->pathCost, node->depth, actions[node->action]);
}

void printFromGoal(Node* node){
    printf("\n---\nBACKTRACKING FROM GOAL:");
    printf("\n");
    char* actions[5] = {"SUCK", "DOWN", "UP", "RIGHT", "LEFT"};
    while (node != NULL) {
        //printWorld(node);
        if(node->pathCost == 0.0 && node->parent == NULL){
                printf("ROOT NODE\n\n\n");
        }
        else{
            printf("action: %s, pathCost: %.1f\n",actions[node->action], node->pathCost);
        }
        node = node->parent;
    }
}

// Function to perform uniform cost tree search
Node* uniformCostTreeSearch(Node* rootNode){
    printf("UNIFORM COST TREE SEARCH:\n");
    
    int totalGenerated = 0;
    int totalExpanded = 0;
    clock_t t;
    clock_t t2;
    t = clock();
    double time_taken;
    
    //initialize the search tree using the root node
    //set the starting fringe (set of reachable but unexpanded nodes)
    Fringe* fringe = createFringe(100000); // Adjust capacity as needed
    insert(fringe, rootNode);
    
    bool finished = false;
    Node* currentNode = NULL;
    //loop do
    while (!finished){
        //if there are no candidates for expansion/fringe is empty: return NULL
        if (fringe->size == 0){return NULL;}
        //choose a leaf node for expansion according to strategy, remove it from the fringe
        //STRATEGY: always expand the least-code node in the fringe. If there's a tie, choose the node in the lowest row number. if there's multiple in said row, choose the node in the lowest column number.
        currentNode = extractMin(fringe, true);
        //if the node contains a goal state: return corresponding solution (string of parent->child nodes)
        if (isGoalState(currentNode->state)) {
            finished = true;
            totalExpanded++;
            t2 = clock() - t;
            time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
            printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
            
            printFromGoal(currentNode);
            
            //free the space
            free(fringe->array);
            free(fringe);
            break;
        }
        //else: expand the node and update the fringe to include any new discovered nodes
        int numSuccessors;
        Node** successors = expandNode(currentNode, &numSuccessors, false);
        totalGenerated += numSuccessors;
        totalExpanded++;
        for (int i = 0; i < numSuccessors; i++) {
            insert(fringe, successors[i]);
        }
        free(successors);
        
        if (debug || totalExpanded < 7){printWorld(currentNode);}
        else{
            if (totalExpanded % 1000 == 0)
            {
                t2 = clock() - t;
                time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
                printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
                if (time_taken >= 3600.0){
                    printf("an hour has elapsed. give up.\n");

                    printFromGoal(currentNode);
                    //free the space
                    free(fringe->array);
                    free(fringe);
                    free(closed->array);
                    free(closed);
                }
            }
            
        }
    }//end
    return currentNode;
}

// Function to perform uniform cost graph search
Node* uniformCostGraphSearch(Node* rootNode){
    printf("UNIFORM COST GRAPH SEARCH:\n");
    int totalGenerated = 0;
    int totalExpanded = 0;
    clock_t t;
    clock_t t2;
    t = clock();
    double time_taken;
    
    //initialize the search tree using the root node
    //set the starting fringe (set of reachable but unexpanded nodes)
    Fringe* fringe = createFringe(1000); // Adjust capacity as needed
    insert(fringe, rootNode);
    Fringe* closed = createFringe(1000);
    
    bool finished = false;
    Node* currentNode = NULL;
    //loop do
    while (!finished){
        //if there are no candidates for expansion/fringe is empty: return NULL
        if (fringe->size == 0){return NULL;}
        //choose a leaf node for expansion according to strategy, remove it from the fringe
        //STRATEGY: always expand the least-code node in the fringe. If there's a tie, choose the node in the lowest row number. if there's multiple in said row, choose the node in the lowest column number.
        currentNode = extractMin(fringe, true);
        //if the node contains a goal state: return corresponding solution (string of parent->child nodes)
        if (isGoalState(currentNode->state)) {
            finished = true;
            totalExpanded++;
            t2 = clock() - t;
            time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
            printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
            
            printFromGoal(currentNode);
            
            //free the space
            free(fringe->array);
            free(fringe);
            free(closed->array);
            free(closed);
            
            break;
        }
        //else: expand the node and update the fringe to include any new discovered nodes IF NOT CLOSED
        //check if the node is closed
        bool valid = true;
        for (int i = 0; i < closed->size; i++){
            if(nodesEqual(currentNode,closed->array[i])){
                if (debug){
                    printf("these nodes are equal! Get it out!\n");
                    printWorld(currentNode);
                    printWorld(closed->array[i]);
                    printf("see? the same!\n");
                }
                valid = false;
                break;
            }
        }
        
        if (valid){
            if (debug){printf("the node wasn't closed!\n");}
            int numSuccessors;
            Node** successors = expandNode(currentNode, &numSuccessors, false);
            totalGenerated += numSuccessors;
            totalExpanded++;
            for (int i = 0; i < numSuccessors; i++) {
                insert(fringe, successors[i]);
            }
            free(successors);
            if (debug || totalExpanded < 7){printWorld(currentNode);}
            else{
                if (totalExpanded % 1000 == 0)
                {
                    t2 = clock() - t;
                    time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
                    printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
                    if (time_taken >= 3600.0){
                        printf("an hour has elapsed. give up.\n");

                        printFromGoal(currentNode);
                        //free the space
                        free(fringe->array);
                        free(fringe);
                        free(closed->array);
                        free(closed);
                    }
                }
            }
            insert(closed, currentNode);
        }
        
    }//end
    return currentNode;
}

// Function to perform iterative deepening tree search
Node* iterativeDeepeningTreeSearch(Node* rootNode){
    printf("ITERATIVE DEEPENING TREE SEARCH:\n");
    
    int totalGenerated = 0;
    int totalExpanded = 0;
    clock_t t;
    clock_t t2;
    t = clock();
    double time_taken;
    
    //initialize the search tree using the root node
    //set the starting fringe (set of reachable but unexpanded nodes)
    Fringe* fringe = createFringe(100000); // Adjust capacity as needed
    insert(fringe, rootNode);
    
    bool finished = false;
    Node* currentNode = NULL;
    //loop do
    while (!finished){
        //if there are no candidates for expansion/fringe is empty: return NULL
        if (fringe->size == 0){return NULL;}
        //choose a leaf node for expansion according to strategy, remove it from the fringe
        //STRATEGY: always expand the lowest depth node in the fringe. If there's a tie, choose the node in the lowest row number. if there's multiple in said row, choose the node in the lowest column number.
        currentNode = extractMin(fringe, false);
        //if the node contains a goal state: return corresponding solution (string of parent->child nodes)
        if (isGoalState(currentNode->state)) {
            finished = true;
            totalExpanded++;
            t2 = clock() - t;
            time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
            printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
            
            printFromGoal(currentNode);
            
            //free the space
            free(fringe->array);
            free(fringe);
            break;
        }
        //else: expand the node and update the fringe to include any new discovered nodes
        int numSuccessors;
        Node** successors = expandNode(currentNode, &numSuccessors, false);
        totalGenerated += numSuccessors;
        totalExpanded++;
        for (int i = 0; i < numSuccessors; i++) {
            insert(fringe, successors[i]);
        }
        free(successors);
        
        if (debug || totalExpanded < 7){printWorld(currentNode);}
        else{
            if (totalExpanded % 1000 == 0)
            {
                t2 = clock() - t;
                time_taken = ((double)t2)/CLOCKS_PER_SEC; // calculate the elapsed time
                printf("\n---\ntotGen: %d, totExp: %d, time: %f\n", totalGenerated, totalExpanded, time_taken);
                if (time_taken >= 3600.0){
                    printf("an hour has elapsed. give up.\n");

                    printFromGoal(currentNode);
                    //free the space
                    free(fringe->array);
                    free(fringe);
                    free(closed->array);
                    free(closed);
                }
            }
            
        }
    }//end
    return currentNode;
}

void main() {
    printf("give me a mode! 1: ucts. 2: ucgs. 3: idts.");
    int mode;
    scanf("%d", &mode);
    printf("give me an instance! 1-2: assignment. 3: my custom instance.");
    int instance;
    scanf("%d", &instance);
    Node* myNode = initializeWorld(instance);
    switch(mode){
        case 1:
            myNode = uniformCostTreeSearch(myNode);
            break;
        case 2:
            myNode = uniformCostGraphSearch(myNode);
            break;
        case 3:
            myNode = iterativeDeepeningTreeSearch(myNode);
            break;
    }
}
