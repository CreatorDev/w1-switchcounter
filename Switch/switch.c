#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <letmecreate/letmecreate.h>
#include <awa/static.h>

#define COUNTER_INSTANCES       (1)
#define OBJECT_INSTANCE_ID      (0)


/**Define DeviceDetails Object**/
typedef struct
{
    char Manufacturer[64];

} DeviceDetailsObject;

static DeviceDetailsObject device[1];

static void DefineDeviceDetailsObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_DefineObject(awaClient, 3, "DeviceDetails", 0, 1);
    AwaStaticClient_DefineResource(awaClient, 3, 0, "Manufacturer", AwaResourceType_String, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 3, 0, &device[0].Manufacturer, sizeof(device[0].Manufacturer), sizeof(device[0]));
}

/**Define SwitchCounter Object**/
typedef struct
{
    int Totalcount;

} SwitchCounterObject;

static SwitchCounterObject counter[COUNTER_INSTANCES];

static void DefineSwitchCounterObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_DefineObject(awaClient, 3200, "Digital Input", 0, COUNTER_INSTANCES);
    AwaStaticClient_DefineResource(awaClient, 3200, 5501, "Counter",  AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 3200, 5501, &counter[0].Totalcount, sizeof(counter[0].Totalcount), sizeof(counter[0]));
}

/**Give Resources a starting value**/
static void SetInitialValues(AwaStaticClient * awaClient)
 {
    AwaStaticClient_CreateObjectInstance(awaClient, 3200, OBJECT_INSTANCE_ID);
    AwaStaticClient_CreateObjectInstance(awaClient, 3, OBJECT_INSTANCE_ID);

    AwaStaticClient_CreateResource(awaClient, 3, OBJECT_INSTANCE_ID, 0);
    strcpy(device[OBJECT_INSTANCE_ID].Manufacturer, "Creator Digital Input");

    AwaStaticClient_CreateResource(awaClient, 3200, OBJECT_INSTANCE_ID, 5501);
    counter[OBJECT_INSTANCE_ID].Totalcount = 0;
}

bool ReadCertificate(const char *filePath, uint8_t **certificate, uint32_t *certificate_length)
{
    FILE *inputFile = fopen(filePath, "rb");
    if (inputFile == NULL)
    {
        printf("Unable to open certificate file under: %s", filePath);
        return false;
    }
    if (fseek(inputFile, 0, SEEK_END) != 0)
    {
        fclose(inputFile);
        printf("Can't set file offset.");
        return false;
    }
    *certificate_length = ftell(inputFile);
    rewind(inputFile);
    *certificate = malloc(*certificate_length * (sizeof(uint8_t)));
    if (fread(*certificate, sizeof(uint8_t), *certificate_length, inputFile) != *certificate_length)
    {
        printf("Could not read certificate file");
        fclose(inputFile);
        return false;
    }
    if (fclose(inputFile) == EOF)
    {
        printf("Couldn't close certificate file.");
        return false;
    }
    return true;
}

static AwaStaticClient * awaClient = NULL;

/**Switch Callback Function**/
static int currentcount = 0;

static void addcount(void)
{
    currentcount++;
    counter[0].Totalcount = currentcount;
    AwaStaticClient_ResourceChanged(awaClient, 3200, 0, 5501);
}

static volatile bool running = true;

static void exit_program()
{
    running = false;
}

int main(void)
{
    char *clientName = "Creator Digital Input";
    /* Set signal handler to exit program when Ctrl+c is pressed */
    struct sigaction action = {
        .sa_handler = exit_program,
        .sa_flags = 0
    };
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, NULL);

    srand(time(NULL));
    int port = 6000 + (rand() % 32768);
 
    uint32_t certificate_length = 0;
    uint8_t *cert = NULL;
    char *certFilePath = "/etc/config/creatorworkshop.crt"; // Path to Certificate file

    awaClient = AwaStaticClient_New();

    AwaStaticClient_SetLogLevel(AwaLogLevel_Verbose);
    AwaStaticClient_SetEndPointName(awaClient, clientName);

    ReadCertificate(certFilePath, &cert, &certificate_length);
    AwaStaticClient_SetCertificate(awaClient, cert, certificate_length, AwaSecurityMode_Certificate);
    
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", port);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coaps://deviceserver.creatordev.io:15684");

    AwaStaticClient_Init(awaClient);

    DefineSwitchCounterObject(awaClient);
    DefineDeviceDetailsObject(awaClient);
    SetInitialValues(awaClient);

    /**Init Switch and Callback**/
    switch_init();
    switch_add_callback(SWITCH_1_PRESSED, addcount);

    while (running)
    {
        AwaStaticClient_Process(awaClient);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}