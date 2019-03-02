/************************************************************************
 * CommunicationManager class
 *
 * Author:  Daniel Stümke
 * Date:    19.02.2019
 *
 */
 #ifndef __COMMUNICATION_MANAGER_H__
 #define __COMMUNICATION_MANAGER_H__

 #ifndef COMMUNICATION_TEST_ENV
 #include "FlexCAN.h"
 #endif

  /************************************************************************
   * Debug Macros and definitions
   */

 #define COMMUNICATION_DEBUG_MODE 0

 #define COMMUNICATION_DEBUG_PRINT(...) if(1 == COMMUNICATION_DEBUG_MODE) Serial.print(__VA_ARGS__)
 #define COMMUNICATION_DEBUG_PRINTLN(...) if(1 == COMMUNICATION_DEBUG_MODE) Serial.println(__VA_ARGS__)


 enum COMMUNICATION_CYCLE { CYCLE_10 = 0, CYCLE_20, CYCLE_40, CYCLE_80, CYCLE_100 };

 typedef struct COMMUNICATION_producer_t {
 	unsigned char* ref;
 	unsigned char bytes;
 	unsigned int canId;
 	unsigned char* txFlag;
 	COMMUNICATION_CYCLE cycle;
 } COMMUNICATION_producer_t;

 typedef struct COMMUNICATION_consumer_t {
 	unsigned char* ref;
 	unsigned char bytes;
 	unsigned int canId;
 	unsigned char* rxFlag;
 } COMMUNICATION_consumer_t;

 typedef struct COMMUNICATION_listNode_t {
 	COMMUNICATION_producer_t producer;
 	COMMUNICATION_listNode_t* left;
 	COMMUNICATION_listNode_t* right;
 } COMMUNICATION_listNode_t;

 #define COMMUNICATION_MAX_CONSUMERS 128
 #define COMMUNICATION_MAX_PRODUCERS 128
 #define COMMUNICATION_FIRE_STACK_SIZE 8
 #define COMMUNICATION_MAX_LIST_NODES 96


 enum COMMUNICATION_BYTE_ORDER { ORDER_MSB, ORDER_LSB };

 class CommunicationManager {
 private:
 	CommunicationManager();

 #ifndef COMMUNICATION_TEST_ENV
 	CAN_message_t CAN_outMsg;
 	CAN_message_t CAN_inMsg;

 	FlexCAN CAN_Can0;
 	CAN_filter_t CAN_filter;
 #endif

 	void InitCan(uint32_t baud);
 	int SendCanMessage(uint16_t msgID, uint8_t *data, uint8_t lengthOfData);
 	int ReceiveCanMessage(uint32_t *id, uint8_t *buf, int n);

 	COMMUNICATION_listNode_t nodes[COMMUNICATION_MAX_LIST_NODES];
 	COMMUNICATION_listNode_t* freeNodes[COMMUNICATION_MAX_LIST_NODES];

 	unsigned int nNodes;
 	unsigned int maxNodesUsed;

 	void InitNodes();
 	COMMUNICATION_listNode_t* NewNode();
 	void DeleteNode(COMMUNICATION_listNode_t* node);

 	COMMUNICATION_listNode_t* nodeList;

 	void InitList();
 	bool ListAdd(COMMUNICATION_producer_t producer);
 	void ListAdd(COMMUNICATION_listNode_t* node);
 	COMMUNICATION_producer_t ListGetHead();
 	void ListRemoveHead();
 	bool ListEmpty();

 	COMMUNICATION_producer_t producers[COMMUNICATION_MAX_PRODUCERS];
 	COMMUNICATION_consumer_t consumers[COMMUNICATION_MAX_CONSUMERS];

 	unsigned int nProducers;
 	unsigned int nConsumers;

 	COMMUNICATION_producer_t emergencies[COMMUNICATION_FIRE_STACK_SIZE];

 	unsigned int nEmergencies;

 	COMMUNICATION_BYTE_ORDER byteOrder;

 public:
 	static CommunicationManager* GetInstance();

 	void Initialize(uint32_t baud = 500000, COMMUNICATION_BYTE_ORDER byteOrder = ORDER_MSB);

 	unsigned int GetMessageUtilization();

 	unsigned int GetMaxMessageUtilization();

 	bool Fire(unsigned int canId);

 	bool Fire(void* val, unsigned int bytes, unsigned int canId);

 	bool Publish(void* val, unsigned int bytes, unsigned int canId, unsigned char* txFlag, COMMUNICATION_CYCLE cycle);

 	bool Subscribe(void* val, unsigned int bytes, unsigned int canId, unsigned char* rxFlag);

 	void Update();
 };

 #endif
