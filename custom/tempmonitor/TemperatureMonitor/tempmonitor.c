#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <letmecreate/letmecreate.h>
#include <awa/static.h>

#define SENSOR_INSTANCES 1

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

/**Define TempSensor Object**/
typedef struct
{
    AwaFloat Temperature;

} TempSensorObject;

static TempSensorObject sensor[SENSOR_INSTANCES];

static void DefineTempSensorObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_DefineObject(awaClient, 3303, "TemperatureSensor", 0, SENSOR_INSTANCES);
    AwaStaticClient_DefineResource(awaClient, 3303, 5700, "Temperature",  AwaResourceType_Float, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 3303, 5700, &sensor[0].Temperature, sizeof(sensor[0].Temperature), sizeof(sensor[0]));
}

/**Give Resources a starting value**/
static void SetInitialValues(AwaStaticClient * awaClient)
 {
    int instance = 0;
 
    AwaStaticClient_CreateObjectInstance(awaClient, 3303, instance);
    AwaStaticClient_CreateObjectInstance(awaClient, 3, instance);
 
    AwaStaticClient_CreateResource(awaClient, 3, instance, 0);
    strcpy(device[instance].Manufacturer, "Creator Temperature Sensor");
 
    AwaStaticClient_CreateResource(awaClient, 3303, instance, 5700);
    sensor[instance].Temperature = 0.0;
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

int main(void)
{
    char *clientName = "Creator Temperature Sensor";
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
 
    AwaStaticClient * awaClient = AwaStaticClient_New();
 
    AwaStaticClient_SetLogLevel(AwaLogLevel_Verbose);
    AwaStaticClient_SetEndPointName(awaClient, clientName);
    AwaStaticClient_SetPSK(awaClient, clientIdentity, clientKeyBinary, keyLength);
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", port);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coaps://deviceserver.creatordev.io:15684");
 
    AwaStaticClient_Init(awaClient);
 
    DefineTempSensorObject(awaClient);
    DefineDeviceDetailsObject(awaClient);
    SetInitialValues(awaClient);

    int instance = 0;
    i2c_init();
	i2c_select_bus(MIKROBUS_1);

    while (1)
    {
        AwaStaticClient_Process(awaClient);
	    
	    //LetMeCreate Thermo3 Click Read
        float temperature = 0.f;

	    thermo3_click_enable(0);
	    thermo3_click_get_temperature(&temperature);
	
        //Update LWM2M resource
        sensor[instance].Temperature = temperature;
        AwaStaticClient_ResourceChanged(awaClient, 3303, 0, 5700);

        //Delay between temperature readings
        sleep(2);           
    }
    
    AwaStaticClient_Free(&awaClient);
    i2c_release();

    return 0;
}
