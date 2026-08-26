#include "codexion.h"

int heap_init(t_heap *h, int capacity)
{
    h->data = malloc(sizeof(t_req) * capacity);
    if (!h->data)
        return (0);
    h->size = 0;
    h->capacity = capacity;
    return (1);
}

void heap_free(t_heap *h)
{
    if (h->data)
    {
        free(h->data);
        h->data = NULL;
    }
    h->size = 0;
}

static int compare_req(t_req a, t_req b, t_scheduler sched)
{
    if (a.priority_key != b.priority_key)
        return (a.priority_key < b.priority_key ? -1 : 1);
    if (sched == EDF)
    {
        if (a.arrival_time != b.arrival_time)
            return (a.arrival_time < b.arrival_time ? -1 : 1);
    }
    return (a.coder_id < b.coder_id ? -1 : 1);
}

static void swap_req(t_req *a, t_req *b)
{
    t_req tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

int heap_push(t_heap *h, t_req req, t_scheduler sched)
{
    int current;
    int parent;

    if (h->size >= h->capacity)
        return (0);
    current = h->size++;
    h->data[current] = req;
    while (current > 0)
    {
        parent = (current - 1) / 2;
        if (compare_req(h->data[current], h->data[parent], sched) < 0)
        {
            swap_req(&h->data[current], &h->data[parent]);
            current = parent;
        }
        else
            break;
    }
    return (1);
}

int heap_peek(t_heap *h, t_req *out)
{
    if (h->size == 0)
        return (0);
    *out = h->data[0];
    return (1);
}

static void heapify_down(t_heap *h, int index, t_scheduler sched)
{
    int left;
    int right;
    int smallest;

    while (1)
    {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;
        if (left < h->size && compare_req(h->data[left], h->data[smallest], sched) < 0)
            smallest = left;
        if (right < h->size && compare_req(h->data[right], h->data[smallest], sched) < 0)
            smallest = right;
        if (smallest != index)
        {
            swap_req(&h->data[index], &h->data[smallest]);
            index = smallest;
        }
        else
            break;
    }
}

void heap_remove_id(t_heap *h, int coder_id, t_scheduler sched)
{
    int i;

    i = 0;
    while (i < h->size)
    {
        if (h->data[i].coder_id == coder_id)
        {
            h->data[i] = h->data[--h->size];
            heapify_down(h, i, sched);
            break;
        }
        i++;
    }
}