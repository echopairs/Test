// test queue and callback
#include "queue.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace std;

TEST(TickTaskQueue, NumberInput)
{
     TickTaskQueue<int> q;
     int a[10] = { 1,4,7,2,5,8,3,6,9,0 };
     for (int i = 0; i < 10; i++)
     {
         q.push(a[i]);
     }
    
     for(int i = 0; i < 10; i++)
     {
         int x;
         q.pop(&x);
         EXPECT_EQ(x,i);  
     }
}


TEST(TickTaskQueue, SpecializedCompare)
{
    TickTaskQueue<long> q;

    long a[10] = { 1,4,7,2,5,8,3,6,9,0 };

    for (long i = 0; i < 10; i++)
    {
        q.push(a[i]);
    }

    for (long i = 0; i < 10; i++)
    {
        long x;
        q.pop(&x);
        EXPECT_EQ(x, i);
    }
}

static bool lowWaterCalled = false;
static bool highWaterCalled = false;
static void *userdata = NULL;

void handleWarterLevel(TickTaskQueueEventType event, void *data)
{
    EXPECT_EQ(userdata, data);
    if (event == TickTaskQueueEventEnterLowLevel)
    {
        lowWaterCalled = true;
    }
    if (event == TickTaskQueueEventEnterHighLevel)
    {
        highWaterCalled = true;
    }

}


TEST(TickTaskQueue, Callback)
{
    TickTaskQueue<int> q(10, 30);
    q.registEventCallback(TickTaskQueueEventEnterLowLevel, handleWarterLevel, userdata);
    q.registEventCallback(TickTaskQueueEventEnterHighLevel, handleWarterLevel, userdata);

    lowWaterCalled = false;
    highWaterCalled = false;
    for (int i = 0; i < 29; i++)
    {
        int x = i;
        q.push(x);
    }

    EXPECT_FALSE(lowWaterCalled);
    EXPECT_FALSE(highWaterCalled);
    
    for (int i = 0; i < 2; i++)
    {
        int x = i + 31;
        q.push(x);
    }
    
    EXPECT_FALSE(lowWaterCalled);
    EXPECT_TRUE(highWaterCalled);
    
    for (int i = 0; i < 21; i++)
    {
        q.pop(NULL);
    }

    EXPECT_TRUE(lowWaterCalled);
    EXPECT_TRUE(highWaterCalled);

    // now count == 10

}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
