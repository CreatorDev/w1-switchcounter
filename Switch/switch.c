#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <letmecreate/letmecreate.h>
#include <awa/static.h>

#define COUNTER_INSTANCES 1

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
    int instance = 0;
 
    AwaStaticClient_CreateObjectInstance(awaClient, 3200, instance);
    AwaStaticClient_CreateObjectInstance(awaClient, 3, instance);
 
    AwaStaticClient_CreateResource(awaClient, 3, instance, 0);
    strcpy(device[instance].Manufacturer, "Creator Digital Input");
 
    AwaStaticClient_CreateResource(awaClient, 3200, instance, 5501);
    counter[instance].Totalcount = 0;
}

uint8_t HexToByte(const char *value)
{
    uint8_t result = 0;
    if (value)
    {
        int count = 2;
        while (count)
        {
            int hex = *value++;
            if (hex >= '0' && hex <= '9')
                result |= hex - '0';
            else if (hex >= 'A' && hex <= 'F')
                result |= 10 + (hex - 'A');
            else if (hex >= 'a' && hex <= 'f')
                result |= 10 + (hex - 'a');
            count--;
            if (count > 0)
                result <<= 4;
        }
    }
    return result;
}

/**Switch Callback Function**/
static AwaStaticClient * awaClient = NULL;
static int currentcount = 0;

static void addcount(void)
{
    currentcount++;
    counter[0].Totalcount = currentcount;
    AwaStaticClient_ResourceChanged(awaClient, 3200, 0, 5501);
}

int main(void)
{
    char *clientName = "Creator Digital Input";
    char *clientIdentity = "";
    char *clientSecretHex = "";
    if (!clientName || !clientIdentity || !clientSecretHex) {
        printf("Please specify CLIENT_NAME, CLIENT_IDENTITY and CLIENT_SECRET environment variables\n");
        exit(2);
    }
 
    int hexKeyLength = strlen(clientSecretHex);
 
    if (hexKeyLength % 2 > 0) {
        printf("Invalid key\n");
        exit(2);
    }
    int keyLength = hexKeyLength / 2;
    char *clientKeyBinary = (uint8_t *)malloc(keyLength);
    if (clientKeyBinary)
    {
        char * value = clientSecretHex;
        int index;
        for (index = 0; index < keyLength; index++)
        {
            clientKeyBinary[index] = HexToByte(value);
            value += 2;
        }
    }
 
    srand(time(NULL));
    int port = 6000 + (rand() % 32768);
 
    awaClient = AwaStaticClient_New();
 
    AwaStaticClient_SetLogLevel(AwaLogLevel_Verbose);
    AwaStaticClient_SetEndPointName(awaClient, clientName);
    AwaStaticClient_SetPSK(awaClient, clientIdentity, clientKeyBinary, keyLength);
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", port);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coaps://deviceserver.creatordev.io:15684");
 
    AwaStaticClient_Init(awaClient);
 
    DefineSwitchCounterObject(awaClient);
    DefineDeviceDetailsObject(awaClient);
    SetInitialValues(awaClient);

    /**Init Switch and Callback**/
    switch_init();
    switch_add_callback(SWITCH_1_PRESSED, addcount);

    while (1)
    {
        AwaStaticClient_Process(awaClient);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}