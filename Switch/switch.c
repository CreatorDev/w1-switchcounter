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
    strcpy(device[instance].Manufacturer, "Imagination Technologies");
 
    AwaStaticClient_CreateResource(awaClient, 3200, instance, 5501);
    counter[instance].Totalcount = 0;
}

bool ReadCertificate(const char *filePath, char **certificate)
{
    size_t inputFileSize;
    FILE *inputFile = fopen(filePath, "rb");
    if (inputFile == NULL)
    {
        printf("Unable to open certificate file under: %s", filePath);
        return false;
    }
    if (fseek(inputFile, 0, SEEK_END) != 0)
    {
        printf("Can't set file offset.");
        return false;
    }
    inputFileSize = ftell(inputFile);
    rewind(inputFile);
    *certificate = malloc(inputFileSize * (sizeof(char)));
    fread(*certificate, sizeof(char), inputFileSize, inputFile);
    if (fclose(inputFile) == EOF)
    {
        printf("Couldn't close certificate file.");
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

int main(void)
{
    char *clientName = "Creator Digital Input";
 
    srand(time(NULL));
    int port = 6000 + (rand() % 32768);
 
    char *cert = NULL;
    char *certFilePath = "/etc/config/creatorworkshop.crt"; // Path to Certificate file

    awaClient = AwaStaticClient_New();
    
    AwaStaticClient_SetLogLevel(AwaLogLevel_Verbose);
    AwaStaticClient_SetEndPointName(awaClient, clientName);

    ReadCertificate(certFilePath, &cert);
    AwaStaticClient_SetCertificate(awaClient, cert, strlen(cert), AwaSecurityMode_Certificate);
    
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