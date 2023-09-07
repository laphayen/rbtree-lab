#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>

void rbtee_right_rotate(rbtree* t, node_t* x);
void rbtee_left_rotate(rbtree* t, node_t* x);
void rbtee_post_order(rbtree* t, node_t* p);
void insert_fixup(rbtree* t, node_t* x);
void transplant(rbtree* t, node_t* u, node_t* v);
void erase_fixup(rbtree* t, node_t* x);
node_t* find_successor(rbtree* t, node_t* p);
void rbtree_in_order(const rbtree* t, node_t* p, key_t* arr, const size_t n, int* cnt);

// RB Tree 구조체 생성
rbtree* new_rbtree(void) {
    // 트리를 자유저장소에서 생성한다.
    rbtree* p = (rbtree*)calloc(1, sizeof(rbtree));
    // calloc으로 메모리 공간 할당 실패 시 NULL값을 반환한다.
    if (p == NULL)
    {
        // 실패 시 처리할 예외
        return NULL;
    }

    // nil 노드를 자유저장소에서 생성한다.
    node_t* nil = (node_t*)calloc(1, sizeof(node_t));
    // calloc으로 메모리 공간 할당 실패 시 NULL값을 반환한다.
    if (nil == NULL)
    {
        // 실패 시 처리할 예외
        free(p);
        return NULL;
    }

    // 루트 노드는 블랙이므로 지정해준다.
    nil->color = RBTREE_BLACK;

    // nil노드를 연결한다
    // 노드의 색상에 상관 없이 
    // "모든 잎은 검은색이다"를 항상 만족시킬 수 있다.
    nil->key = 0;
    nil->parent = NULL;
    nil->left = NULL;
    nil->right = NULL;

    p->root = nil;
    p->nil = nil;

    // 생성된 포인터를 반환한다.
    return p;
}

// 우회전
// 부모-자식 노드의 위치를 서로 바꾸는 연산이다.
// 왼쪽 자식과 부모의 위치를 교환하는 것.
void rbtee_right_rotate(rbtree* t, node_t* x) {
    // x의 왼쪽 자식노드인 x_child 설정
    node_t* x_child = x->left;

    // 왼쪽 자식 노드의 오른쪽 자식 노드를
    // 부모 노드의 왼쪽 자식으로 등록한다.
    x->left = x_child->right;

    // 오른쪽 자식 노드가 nil 노드가 아닐 경우
    if (x_child->right != t->nil)
        // 오른쪽 자식 노드의 부모 노드를 x로 변경한다.
        x_child->right->parent = x;

    // x_child의 부모 노드를 x의 부모 노드로 변경한다.
    x_child->parent = x->parent;

    // 부모가 NULL이라면 루트이기 때문에
    // 왼쪽 자식을 루트로 만들어서 우회전을 시킨다.
    if (x->parent == t->nil)
        t->root = x_child;
    // x가 x의 부모 노드의 오른쪽 자식 노드일 경우
    // x의 보모 노드의 오른쪽 자식 노드를 x_child로 변경한다.
    else if (x == x->parent->left)
        x->parent->left = x_child;
    // x가 x의 부모 노드의 왼쪽 자식 노드이면
    // x의 부모 노드의 왼쪽 자식 노드를 x_child로 변경한다.
    else
        x->parent->right = x_child;
    
    // x_child의 오른쪽 자식 노드를 x로 변경한다.
    x_child->right = x;
    // x의 부모 노드를 x_child로 변경한다.
    x->parent = x_child;
}

// 좌회전
// 부모-자식 노드의 위치를 서로 바꾸는 연산이다.
// 오른쪽 자식과 부모의 위치를 교환하는 것.
void rbtee_left_rotate(rbtree* t, node_t* x) {
    node_t* x_child = x->right;

    // 오른쪽 자식 노드의 왼쪽 자식 노드를
    // 부모 노드의 오른쪽 자식으로 등록한다.
    x->right = x_child->left;

    // x_child의 왼쪽 자식 노드가 nil이 아닐 경우
    // x_child의 왼쪽 자식 노드의 부모를 x로 변경한다.
    if (x_child->left != t->nil)
        x_child->left->parent = x;

    // x_child의 부모 노드를 x의 부모 노드로 변경한다.
    x_child->parent = x->parent;

    // 부모가 NULL이라면 루트이기 때문에
    // 오른쪽 자식을 루트로 만들어서 좌회전을 시킨다.
    if (x->parent == t->nil)
        t->root = x_child;
    // x가 x의 부모 노드의 왼쪽 자식일 경우
    // x의 부모 노드의 왼쪽 자식 노드를 x_child로 변경한다.
    else if (x == x->parent->left)
        x->parent->left = x_child;
    // x가 x의 부모 노드의 오른쪽 자식 노드이면
    // x의 부모 노드의 오른쪽 자식 노드를 x_child로 변경한다.
    else x->parent->right = x_child;

    // x_child의 왼쪽 자식을 x로 변경한다.
    x_child->left = x;
    // x의 부모 노드를 x_child로 변경한다.
    x->parent = x_child;
}

// 후위 순회를 통해 노드를 삭제한다.
// 부모 노드를 삭제하기 전에 자식 노드를 삭제하는 순으로
// 노드를 삭제하기 때문이다.
// 왼쪽 서브 트리 -> 오른쪽 서브 트리 -> 루트 노드 순
void rbtee_post_order(rbtree* t, node_t* p) {
    if (p == t->nil)
        return;

    // 왼쪽 노드 탐색
    rbtee_post_order(t, p->left);
    // 오른쪽 노드 탐색
    rbtee_post_order(t, p->right);
    // 메모리 해제
    free(p);
}

// RB Tree의 모든 메모리를 반환한다.
void delete_rbtree(rbtree* t) {
    // 후위 순회를 통해 메모리를 해제한다.
    rbtee_post_order(t, t->root);
    // nill 노드의 메모리를 해제한다.
    free(t->nil);
    // RB Tree의 메모리를 해제한다.
    free(t);
}

// 삽입으로 인한 RB Tree의 조건을 만족시킨다.
void insert_fixup(rbtree* t, node_t* x) {
    // 빨간 노드의 자식은 빨 (4번 규칙)
    // 삽입한 노드가 빨이면 부모가 빨이면 실행.
    while (x->parent->color == RBTREE_RED) {
        if (x->parent == x->parent->parent->left) {
            node_t* uncle = x->parent->parent->right;
            // 부모 노드가 빨이고 삼촌이 빨이면
            if (uncle->color == RBTREE_RED) {
                // 조상 노드=빨, 부모삼촌을 검으로 바꿔준다.
                x->parent->color = RBTREE_BLACK;
                uncle->color = RBTREE_BLACK;
                x->parent->parent->color = RBTREE_RED;
                // 조상 노드를 삽입 노드로 간주한다.
                x = x->parent->parent;
            }
            else {
                // 삼촌이 검은색이고 x가 오른쪽 자식일 때
                // 부모 노드를 외쪽으로 회전시켜서
                // 삼촌이 검은색이며, 
                // 새로 삽입한 노드가 부모의 왼쪽 자식인 경우로
                // 풀이를 한다.
                if (x == x->parent->right) {
                    x = x->parent;
                    rbtee_left_rotate(t, x);
                }
                x->parent->color = RBTREE_BLACK;
                x->parent->parent->color = RBTREE_RED;
                rbtee_right_rotate(t, x->parent->parent);
            }
        }
        else if (x->parent == x->parent->parent->right) {
            node_t* uncle = x->parent->parent->left;
            // 삼촌이 빨간색이고
            // 새로삽입한 노드가 부모 노드의 왼쪽 자식인 경우
            if (uncle->color == RBTREE_RED) {
                x->parent->color = RBTREE_BLACK;
                uncle->color = RBTREE_BLACK;
                x->parent->parent->color = RBTREE_RED;
                x = x->parent->parent;
            }
            else {
                // 삼촌이 검은색이고 x가 오른쪽 자식인 경우
                if (x == x->parent->left) {
                    x = x->parent;
                    rbtee_right_rotate(t, x);
                }
                // 삼촌이 검은색이고, x가 왼쪽 자식인 경우
                x->parent->color = RBTREE_BLACK;
                x->parent->parent->color = RBTREE_RED;
                rbtee_left_rotate(t, x->parent->parent);
            }
        }

    }
    t->root->color = RBTREE_BLACK;
}

// 트리에 노드를 삽입한다.
node_t* rbtree_insert(rbtree* t, const key_t key) {
    node_t* parent = t->nil;
    node_t* p = t->root;
    while (p != t->nil) {
        parent = p;
        if (p->key > key) p = p->left;
        else p = p->right;
    }
    // 새로 추가할 노드르르 자유저장소에 할당한다.
    node_t* insertion_node = (node_t*)calloc(1, sizeof(node_t));

    // 삽입 시 노드의 색상은 빨강으로 지정한다.
    insertion_node->color = RBTREE_RED;
    insertion_node->key = key;
    insertion_node->parent = parent;

    // 구현의 편의를 위해 양쪽 노드를 nil 노드로 간주한다.
    insertion_node->left = t->nil;
    insertion_node->right = t->nil;

    if (parent == t->nil)
        t->root = insertion_node;
    else if (parent->key > key)
        parent->left = insertion_node;
    else 
        parent->right = insertion_node;

    // 노드를 삽입한 이후에 조건을 만족시킨다.
    insert_fixup(t, insertion_node);

    return insertion_node;
}

// RB Tree에서 키를 찾는 함수이다.
// 이진 검색 트리에서 키를 찾는 함수와 같다.
node_t* rbtree_find(const rbtree* t, const key_t key) {
    // 루트부터 탐색을 한다.
    node_t* p = t->root;
    while (p != t->nil)
    {
        // 값을 찾을 경우
        if (p->key == key)
            return p;
        // 값이 작을 경우
        if (p->key > key)
            p = p->left;
        // 값이 클 경우
        else
            p = p->right;
    }
    return NULL;
}

// RB Tree 최솟값 찾기
node_t* rbtree_min(const rbtree* t) {
    // 루트부터 탐색을 시작한다.
    node_t* p = t->root;
    // 이진 트리에서 최솟값은 트리의 루트 노드로 부터
    // 가장 깊은 왼쪽 노드에 있으므로
    // 왼쪽 끝까지 탐색하여 마지막 노드의 왼쪽이 
    // nil노드가 나올 때까지 탐색한다.
    while (p->left != t->nil)
    {
        p = p->left;
    }
    // 최솟값을 반환한다.
    return p;
}

// RB Tree 최댓값 찾기 
node_t* rbtree_max(const rbtree* t) {
    // 루트부터 탐색을 시작한다.
    node_t* p = t->root;
    // 이진 트리에서 최댓값은 트리의 루트 노드로 부터
    // 가장 깊은 오른쪽 노드에 있으므로
    // 오른쪽 끝까지 탐색하여 마지막 노드의 오른쪽이 
    // nil노드가 나올때 까지 탐색한다.
    while (p->right != t->nil)
    {
        p = p->right;
    }
    // 최댓값을 반환한다.
    return p;
}

// 삭제 시 삭제될 노드의 부모, 자식 포인터를 전달한다.
void transplant(rbtree* t, node_t* u, node_t* v) {
    if (u->parent == t->nil) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

// 노드 삭제 후 RB Tree의 조건을 만족시킨다.
void erase_fixup(rbtree* t, node_t* x) {
    // 형제 노드
    node_t* target = x;
    // 루트 노드이거나 빨간색 노드에게 검은색이 넘어가면 루프 종료한다.
    while ((target != t->root) && (target->color == RBTREE_BLACK)) {
        if (target == target->parent->left) {
            node_t* w = target->parent->right;
            // case 1 = 형제 노드가 빨강 => 블랙으로 처리한 후에
            if (w->color == RBTREE_RED) {
                w->color = RBTREE_BLACK;
                target->parent->color = RBTREE_RED;
                rbtee_left_rotate(t, target->parent);
                w = target->parent->right;
            }
            // case 2 = 형제가 검은색일때 
            // A. 양쪽 자식이 모두 검은색이면
            if ((w->left->color == RBTREE_BLACK) && (w->right->color == RBTREE_BLACK)) {
                w->color = RBTREE_RED;
                // 부모를 기준으로 잡는다.
                target = target->parent;
            }
            else {
                // B. 왼쪽 자식이 빨간색인 경우
                if (w->right->color == RBTREE_BLACK) {
                    w->left->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    rbtee_right_rotate(t, w);
                    w = target->parent->right;
                }
                // C. 오른쪽 자식이 빨간색인 경우
                w->color = target->parent->color;
                target->parent->color = RBTREE_BLACK;
                w->right->color = RBTREE_BLACK;
                rbtee_left_rotate(t, target->parent);
                target = t->root;
            }
        }
        else {
            node_t* w = target->parent->left;
            // x의 형제가 빨간색 노드인 경우
            if (w->color == RBTREE_RED) {
                w->color = RBTREE_BLACK;
                target->parent->color = RBTREE_RED;
                rbtee_right_rotate(t, target->parent);
                w = target->parent->left;
            }
            // x의 형제가 검은색이고 형제의 자식들이 모두 검은색인 경우
            if ((w->left->color == RBTREE_BLACK) && (w->right->color == RBTREE_BLACK)) {
                // x를 검은색 노드 하나로 만들고
                // 형제는 빨간색으로 변경한다.
                w->color = RBTREE_RED;
                target = target->parent;
            }
            else {
                // x의 형제가 검은색이고 형제의 왼쪽 자식은 검은색이고 오른쪽은 빨간색인 경우
                if (w->left->color == RBTREE_BLACK) {
                    w->right->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    rbtee_left_rotate(t, w);
                    w = target->parent->left;
                }
                // x의 형제가 검은색이고 형제의 왼쪽 자식이 빨간색인 경우
                w->color = target->parent->color;
                target->parent->color = RBTREE_BLACK;
                w->left->color = RBTREE_BLACK;
                rbtee_right_rotate(t, target->parent);
                // x를 루트로 변경한다.
                target = t->root;
            }
        }
    }
    // 루트 노드는 검은색을 만족시켜준다.
    target->color = RBTREE_BLACK;
}

// 오른쪽 서브 트리에서 최솟값을 찾는다.
node_t* find_successor(rbtree* t, node_t* p) {
    while (p->left != t->nil) {
        p = p->left;
    }
    return p;
}

// RB Tree에서 노드를 삭제한다.
int rbtree_erase(rbtree* t, node_t* p) {
    // 노드 y를 복사한 p를 선언한다.
    node_t* y = p;
    // 노드 x 선언한다.
    node_t* x;
    // 노드의 색상을 저장한다.
    color_t y_original_color = y->color;

    // p의 왼쪽에 아무것도 없으면 x를 오른쪽으로 설정하고
    // x를 p의 부모랑 붙인다.
    if (p->left == t->nil) {
        x = p->right;
        transplant(t, p, p->right);
    }
    // p의 오른쪽에 아무것도 없으면
    // x를 왼쪽으로 설정하고
    // x를 p의 부모랑 붙인다.
    else if (p->right == t->nil) {
        x = p->left;
        transplant(t, p, p->left);
    }
    // p 밑에 두 개 있을 경우
    else {
        // p의 오른쪽에서 최솟값을 찾고 y로 설정한다.
        y = find_successor(t, p->right);
        // y의 색상을 저장하고 삭제 시 사용한다.
        y_original_color = y->color;
        // x를 y의 왼쪽으로 설정한다.
        x = y->right;
        if (y->parent == p) x->parent = y;
        else {
            transplant(t, y, y->right);
            y->right = p->right;
            y->right->parent = y;
        }
        transplant(t, p, y);
        y->left = p->left;
        y->left->parent = y;
        y->color = p->color;
    }
    // 삭제한 노드가 검은색인 경우
    // 대체 노드를 erase_fixup() 함수에 매개변수를 넘겨 호출한다.
    if (y_original_color == RBTREE_BLACK) {
        erase_fixup(t, x);
    }
    // 삭제 대상의 노드의 메모리 해제한다.
    free(p);
    return 0;
}


// 이진 트리에서 중위 순회는
// 오름차순 또는 내림차순으로 값을 가져올 때 사용한다.
void rbtree_in_order(const rbtree* t, node_t* p, key_t* arr, const size_t n, int* cnt) {
    if (p == t->nil)
        return;
    rbtree_in_order(t, p->left, arr, n, cnt);
    if (*cnt == n)
        return;
    arr[*cnt] = p->key;
    *cnt += 1;

    rbtree_in_order(t, p->right, arr, n, cnt);
}

// RB Tree에서 오른차순으로 정렬된 배열로 변환한다.
// 배열의 크기는 n이며, RB Tree 보다 크기가 n보다 크면
// 오름차순으로 n개까지만 변환한다.
int rbtree_to_array(const rbtree* t, key_t* arr, const size_t n) {
    int cnt = 0;
    int* p = &cnt;
    rbtree_in_order(t, t->root, arr, n, p);
    return 0;
}