#include <threading.h>
#include <stdio.h>

void t_init()
{
    // TODO
    for (int index = 0; index < NUM_CTX; index++)
    {
        struct worker_context contex; // = (struct worker_context *)malloc( sizeof(struct worker_context));
        // memset(contex, 0, sizeof(struct worker_context));

        contex.state = INVALID;
        contexts[index] = contex;
    }

    contexts[0].state = VALID;
    current_context_idx = 0;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
    // ontexts[current_context_idx].state = VALID;
    //  TODO
    //  code that determines the next available
    volatile uint8_t availableContex;
    for (volatile uint8_t index = 0; index < NUM_CTX; index++)
    {
        if (contexts[index].state == INVALID && index != current_context_idx)
        {
            availableContex = index;
            break;
        }
        else
        {
            availableContex = NUM_CTX;
        }
    }
    if (availableContex == NUM_CTX)
    {
        return 1;
    }

    // create the new contex
    getcontext(&contexts[availableContex].context);
    contexts[availableContex].state = VALID;
    contexts[availableContex].context.uc_stack.ss_sp = (char *)malloc(STK_SZ);
    contexts[availableContex].context.uc_stack.ss_size = STK_SZ;
    contexts[availableContex].context.uc_stack.ss_flags = 0;
    contexts[availableContex].context.uc_link = NULL; // &contexts[current_context_idx].context;
    makecontext(&contexts[availableContex].context, (void (*)())foo, 2, arg1, arg2);
    // printf("Worker Created %d \n", availableContex);

    return 0;
}

int32_t t_yield()
{

    // TODO
    // updating the current context
    // getcontext(&contexts[current_context_idx].context);
    volatile uint8_t availableContex;

    // gets next available contex
    for (volatile uint8_t index = 0; index < NUM_CTX; index++)
    {
        if (contexts[index].state == VALID && index != current_context_idx)
        {
            availableContex = index;
            break;
        }
        else
        {
            availableContex = NUM_CTX;
        }
    }
    if (availableContex == NUM_CTX)
    {
        return 1;
    }
    volatile uint8_t temp;
    //  swapcontext
    temp = current_context_idx;
    current_context_idx = availableContex;
    // printf("Current avaiable %d \n", availableContex);
    // printf("Current Context %d \n", temp);
    swapcontext(&contexts[temp].context, &contexts[availableContex].context);

    // swapcontext(&contexts[current_context_idx].context, &contexts[availableContex].context);
    // current_context_idx = availableContex;
    // printf("Current Context %d \n", current_context_idx);

    //    count the number of avaiable context
    int32_t count = -1;
    // cycles through the context and updates them
    for (uint8_t index = 0; index < NUM_CTX; index++)
    {
        if (contexts[index].state == VALID)
        {
            count++;
        }
    }

    // printf("Current count %d \n", count);
    return count;
}

void t_finish()
{
    // TODO

    free(contexts[current_context_idx].context.uc_stack.ss_sp);
    // printf("freed the context %d \n", current_context_idx);

    contexts[current_context_idx].state = DONE;

    // printf("Process is free and done \n");
    t_yield();
}
