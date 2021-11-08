#include "mbed.h"
#include <cstdint>
#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <AdvertisingDataSimpleBuilder.h>
#include <ble/gatt/GattCharacteristic.h>
#include <ble/gatt/GattService.h>
#include "att_uuid.h"

#include <USBSerial.h>

USBSerial ser;

UUID calebs_uuid("calebschip");

Thread tempthread;

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

BLE &bleinit= BLE::Instance();
Gap& gap = bleinit.gap();
GattServer& gattServe = bleinit.gattServer();
GattClient& gattClient = bleinit.gattClient();

int16_t TOUT =0;

using namespace ble;

/**
 * Event handler struct
 */
struct GapEventHandler : Gap::EventHandler{
    
    void onAdvertisingStart (const AdvertisingStartEvent& event) override {
        ser.printf("Advertising Started\r\n");
    }

    void onAdvertisingEnd (const AdvertisingEndEvent& event) override {
        if (event.getStatus() != BLE_ERROR_NONE) {
            ser.printf("Error while ending advertising\r\n");
        }
        else {
            ser.printf("Advertising ending succusseful\r\n");
        }

        if (event.isConnected() == true) {
            ser.printf("Is connected\r\n");
        }
        else {
            ser.printf("Is not connected\r\n");
        }
    }
};


GapEventHandler THE_gap_EvtHandler;


void measure_temp(){
    I2C sensor_bus(I2C_SDA1, I2C_SCL1);

    const int readaddr = 0xBF;
    const int writeaddr = 0xBE;
    uint8_t whoamiaddr[] = {0x0F};
    uint8_t subaddr[1];
    int resp=4;

    char readData[] ={0, 0};
    resp = sensor_bus.write(writeaddr, (const char *) whoamiaddr, 1, true);
    
    if(  resp != 0 ){
        ser.printf("I failed to talk at the temp sensor. (Returned: %d)\n\r", resp);            
    }
              
    if( sensor_bus.read(readaddr, readData, 1)  != 0 ){
        ser.printf("I failed to listen to the temp sensor.\n\r");        
    }
    
    ser.printf("Who Am I? %d\n", readData[0] );
    if( readData[0] != 0xBC ){
        ser.printf("Who are are you?\n\r");
    }

    readData[0] = 0x20; // Control Reg 1
    readData[1] = 0x84; // Turn on our temp sensor, and ensure that we read low to high on our values.
    resp = sensor_bus.write(readaddr, readData, 2);    


    uint8_t databuf[2];
    while(1){        
        readData[0] = 0x21; // Control Reg 2
        readData[1] = 0x1; // Signal a one shot temp reading.
        resp = sensor_bus.write(readaddr, readData, 2);

        
        subaddr[0] = 0x2A; // LSB Temperature
        sensor_bus.write(writeaddr, (const char *) subaddr, 1, true);
        sensor_bus.read(readaddr, readData, 1);
        databuf[0] = ((uint8_t)readData[0]);

        subaddr[0] = 0x2B; // MSB Temperature
        sensor_bus.write(writeaddr, (const char *) subaddr, 1, true);
        sensor_bus.read(readaddr, readData, 1);
        databuf[1] = readData[0];

        TOUT = databuf[0] | (databuf[1]<<8);
        ser.printf("Uncalibrated temperature: %d\n\r",TOUT);

        // Sleep for a while.
        thread_sleep_for(5000);
    }
}

void on_init_complete(BLE::InitializationCompleteCallbackContext *params){
    if(params -> error != BLE_ERROR_NONE) {
        ser.printf("BLE Initialization Failed\r\n");
        return;
    }
    
    gap.setEventHandler(&THE_gap_EvtHandler);

    ble_error_t error1 = gap.setAdvertisingPayload(
        LEGACY_ADVERTISING_HANDLE, 
        AdvertisingDataSimpleBuilder<LEGACY_ADVERTISING_MAX_SIZE>()
            .setFlags()
            .setName("Caleb's Device", true)
            .setAppearance(adv_data_appearance_t::GENERIC_THERMOMETER)
            .getAdvertisingData()
    );
    if(error1 == BLE_ERROR_NONE) {
        ser.printf("Advertising payload set\r\n");
    }
    else {
        ser.printf("Error advertising payload not set\r\n");
    }

    GattCharacteristic* characteristics[] = new gattServer.ReadOnlyGattCharacteristic(calebs_uuid, &TOUT,
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
        nullptr, 0);
    int arrSize = sizeof(characteristics)/sizeof(characteristics[0]);
    GattService service = GattService(calebs_uuid, characteristics, arrSize);
    gattServer.addService(service);

    ble_error_t error = gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if(error != BLE_ERROR_NONE) {
        ser.printf("Error advertising not started\r\n");
    }

    gattServer.write()
}

/* Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context){
    event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}


int main(){
    DigitalOut i2cbuspull(P1_0); // Pull up i2C. resistor.
    i2cbuspull.write(1);
    DigitalOut sensor_pwr(P0_22); // Supply power to all of the sensors (VCC)
    sensor_pwr.write(1);

    bleinit.init(&on_init_complete);
    bleinit.onEventsToProcess(schedule_ble_events);
   

    // This will never return...
    event_queue.dispatch_forever();
}

