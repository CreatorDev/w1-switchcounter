#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#include <letmecreate/letmecreate.h>

#define OPERATION_PERFORM_TIMEOUT   (1000)

static int currentcount;
static AwaClientSession * session;

//Clean exit logic
static volatile bool running = true;

static void exit_program(int __attribute__ ((unused))signo)
{
    running = false;
}

/* Definition of the Digital Input IPSO object */
static void DefineButtonObject(void)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(3200, "Button", 0, 1);
    AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 5501, "Count", false, AwaResourceOperations_ReadOnly, 0);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

/* Create LWM2M Instances and Resources */
static void SetInitialValues(void)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_CreateObjectInstance(operation, "/3200/0");
    AwaClientSetOperation_CreateOptionalResource(operation, "/3200/0/5501");

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

/* Counter callback function that is called on each button press */
static void addcount()
{
    if (!session)
        return;

    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    currentcount++;
    AwaClientSetOperation_AddValueAsInteger(operation, "/3200/0/5501", currentcount);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

int main(void)
{
    /* Set signal handler to exit program when Ctrl+c is pressed */
    struct sigaction action = {
        .sa_handler = exit_program,
        .sa_flags = 0
    };
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, NULL);

    /* Create and initialise client session */
    session = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(session);

    /* Set up LWM2M objects/instances/resources */
    DefineButtonObject();
    SetInitialValues();

    /* Initialise Switch and Callback */
    switch_init();
    switch_add_callback(SWITCH_1_PRESSED, addcount);

    while (running)
        AwaClientSession_Process(session, OPERATION_PERFORM_TIMEOUT);

    /* Cleanly close session */
    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);

    return 0;
}