/************************************************************************
 * CommunicationManager implementation
 *
 * Author:  Daniel Stümke
 * Date:    19.02.2019
 *
 */
#include "Arduino.h"
#include "CommunicationManager.h"
#include "FlexCAN.h"
#include <Metro.h>

static Metro COMMUNICATION_cycle10 = Metro(10);
static Metro COMMUNICATION_cycle20 = Metro(20);
static Metro COMMUNICATION_cycle40 = Metro(40);
static Metro COMMUNICATION_cycle80 = Metro(80);
static Metro COMMUNICATION_cycle100 = Metro(100);

CommunicationManager::CommunicationManager() {
    nProducers = 0;
    nConsumers = 0;
    nEmergencies = 0;
  }

CommunicationManager* CommunicationManager::GetInstance() {
  static CommunicationManager instance;
  return &instance;
}

void CommunicationManager::Initialize(uint32_t baud) {
   InitCan(baud);
   InitNodes();
   InitList();
}

bool CommunicationManager::Fire(unsigned int canId) {
  for(unsigned int i=0; i<nProducers; i++) {
    if(producers[i].canId == canId) {
      return Fire(
        producers[i].ref,
        producers[i].bytes,
        producers[i].canId
      );
    }
  }

  COMMUNICATION_DEBUG_PRINT("[");
  COMMUNICATION_DEBUG_PRINT(millis(), DEC);
  COMMUNICATION_DEBUG_PRINT("] CommunicationManager: in Fire(unsigned int canId=");
  COMMUNICATION_DEBUG_PRINT(canId, HEX);
  COMMUNICATION_DEBUG_PRINTLN(") Unknown Can ID!");

  /* Failed: Can ID unknown */
  return false;
}

bool CommunicationManager::Fire(void* val, unsigned int bytes, unsigned int canId) {
  if(bytes > 8) {
    COMMUNICATION_DEBUG_PRINT("[");
    COMMUNICATION_DEBUG_PRINT(millis(), DEC);
    COMMUNICATION_DEBUG_PRINT("] CommunicationManager: in Fire(void* val, unsigned int bytes, unsigned int canId=");
    COMMUNICATION_DEBUG_PRINT(canId, HEX);
    COMMUNICATION_DEBUG_PRINTLN(") message size truncated to 8 byte!");
    bytes = 8;
  }

  if(COMMUNICATION_FIRE_STACK_SIZE > nEmergencies) {
    emergencies[nEmergencies].ref = (unsigned char*)val;
    emergencies[nEmergencies].bytes = bytes;
    emergencies[nEmergencies].canId = canId;
    nEmergencies += 1;

    /* Success */
    return true;
  }

  COMMUNICATION_DEBUG_PRINT("[");
  COMMUNICATION_DEBUG_PRINT(millis(), DEC);
  COMMUNICATION_DEBUG_PRINT("] CommunicationManager: in Fire(void* val, unsigned int bytes, unsigned int canId=");
  COMMUNICATION_DEBUG_PRINT(canId, HEX);
  COMMUNICATION_DEBUG_PRINTLN(") Emergency queue is full!");

  /* Failed: Emergency stack full */
  return false;
}

bool CommunicationManager::Publish(void* val, unsigned int bytes, unsigned int canId, unsigned char* txFlag, COMMUNICATION_CYCLE cycle) {
  if(bytes > 8) {
    COMMUNICATION_DEBUG_PRINT("[");
    COMMUNICATION_DEBUG_PRINT(millis(), DEC);
    COMMUNICATION_DEBUG_PRINT("] CommunicationManager: in Publish(void* val, unsigned int bytes, unsigned int canId=");
    COMMUNICATION_DEBUG_PRINT(canId, HEX);
    COMMUNICATION_DEBUG_PRINTLN(", unsigned char* txFlag, COMMUNICATION_CYCLE cycle) message size truncated to 8 byte!");
    bytes = 8;
  }

  if(COMMUNICATION_MAX_PRODUCERS > nProducers) {
    producers[nProducers].ref = (unsigned char*)val;
    producers[nProducers].bytes = bytes;
    producers[nProducers].canId = canId;
    producers[nProducers].cycle = cycle;
    producers[nProducers].txFlag = txFlag;
    *txFlag = 0;
    nProducers += 1;

    /* Success */
    return true;
  }

  COMMUNICATION_DEBUG_PRINT("[");
  COMMUNICATION_DEBUG_PRINT(millis(), DEC);
  COMMUNICATION_DEBUG_PRINT("] CommunicationManager: Failed to register Publisher with Can Id ");
  COMMUNICATION_DEBUG_PRINT(canId, HEX);
  COMMUNICATION_DEBUG_PRINTLN(", not enough memory allocated!");

  /* Failed: To many publishers */
  return false;
}

bool CommunicationManager::Subscribe(void* val, unsigned int bytes, unsigned int canId, unsigned char* rxFlag) {
  if(COMMUNICATION_MAX_CONSUMERS > nConsumers) {
    consumers[nConsumers].ref = (unsigned char*)val;
    consumers[nConsumers].bytes = bytes;
    consumers[nConsumers].canId = canId;
    consumers[nConsumers].rxFlag = rxFlag;
    *rxFlag = 0;
    nConsumers += 1;

    /* Success */
    return true;
  }

  COMMUNICATION_DEBUG_PRINT("[");
  COMMUNICATION_DEBUG_PRINT(millis(), DEC);
  COMMUNICATION_DEBUG_PRINT("] CommunicationManager: Failed to register Subscriber with Can Id ");
  COMMUNICATION_DEBUG_PRINT(canId, HEX);
  COMMUNICATION_DEBUG_PRINTLN(", not enough memory allocated!");

  /* Failed: to many subscribers */
  return false;
}

void CommunicationManager::Update() {
  // Handle incomming messages
  uint32_t inId;
  unsigned char inBuf[8];
  while(ReceiveCanMessage(&inId, inBuf, 8)) {
    // Look for consumers
    for(unsigned int i=0; i<nConsumers; i++) {
      if(consumers[i].canId == inId) {
        unsigned char* outData = (uint8_t*)consumers[i].ref;
        for(unsigned int n=0; n<consumers[i].bytes; n++) {
           outData[n] = inBuf[n];
        }
        // Indicate arrival of a message
        *(consumers[i].rxFlag) = 1;
      }
    }
  }

  // Handle emergency messages
  while(nEmergencies > 0) {
    unsigned int i = nEmergencies-1;
    if(!ListAdd(emergencies[i])) {
      COMMUNICATION_DEBUG_PRINT("[");
      COMMUNICATION_DEBUG_PRINT(millis(), DEC);
      COMMUNICATION_DEBUG_PRINT("] CommunicationManager: ");
      COMMUNICATION_DEBUG_PRINT(emergencies[i].canId, HEX);
      COMMUNICATION_DEBUG_PRINTLN(" Queue failed (Emergency)!");
    }
    nEmergencies -= 1;
  }

  // Handle message queuing
  uint8_t cycleFlags[] = {
    COMMUNICATION_cycle10.check(),
    COMMUNICATION_cycle20.check(),
    COMMUNICATION_cycle40.check(),
    COMMUNICATION_cycle80.check(),
    COMMUNICATION_cycle100.check()
    };

  for(unsigned int i=0; i<(sizeof(cycleFlags)/sizeof(cycleFlags[0])); i++) {
    for(unsigned int j=0; j<nProducers; j++){
      if(1 == cycleFlags[(uint32_t)producers[j].cycle]) {
        //unsigned int outId = producers[j].canId;
        //unsigned char* data = producers[j].ref;
        //unsigned char bytes = producers[j].bytes;
        //if(0 == SendCanMessage(outId, data, bytes)) {
        //  COMMUNICATION_DEBUG_PRINT("[");
        //  COMMUNICATION_DEBUG_PRINT(millis(), DEC);
        //  COMMUNICATION_DEBUG_PRINT("] CommunicationManager: ");
        //  COMMUNICATION_DEBUG_PRINT(outId, HEX);
        //  COMMUNICATION_DEBUG_PRINTLN(" Write failed!");
        //}

        // Indicate transmission of a message
        //*(producers[j].txFlag) = 1;
        if(!ListAdd(producers[j])) {
          COMMUNICATION_DEBUG_PRINT("[");
          COMMUNICATION_DEBUG_PRINT(millis(), DEC);
          COMMUNICATION_DEBUG_PRINT("] CommunicationManager: ");
          COMMUNICATION_DEBUG_PRINT(producers[j].canId, HEX);
          COMMUNICATION_DEBUG_PRINTLN(" Queue failed!");
        } else {
          // Indicate transmission of a message
          *(producers[j].txFlag) = 1;
        }
      }
    }
  }

  // Handle message transmission
  if(!ListEmpty()) {
    COMMUNICATION_producer_t producer = ListGetHead();
    unsigned int outId = producer.canId;
    unsigned char* data = producer.ref;
    unsigned char bytes = producer.bytes;
    if(SendCanMessage(outId, data, bytes)) {
      // Free storage
      ListRemoveHead();
    } else {
      //COMMUNICATION_DEBUG_PRINT("[");
      //COMMUNICATION_DEBUG_PRINT(millis(), DEC);
      //COMMUNICATION_DEBUG_PRINT("] CommunicationManager: ");
      //COMMUNICATION_DEBUG_PRINTLN("Send failed!");
    }
  }
}

unsigned int CommunicationManager::GetMessageUtilization(){
  return nNodes;
}

unsigned int CommunicationManager::GetMaxMessageUtilization() {
  return maxNodesUsed;
}

void CommunicationManager::InitCan(uint32_t baud) {
     CAN_Can0 = FlexCAN(baud);
     //Set recive CAN_filter
     CAN_filter.id = 0;
     CAN_filter.ext = 0;
     CAN_filter.rtr = 0;
     CAN_Can0.begin(CAN_filter);
}

int CommunicationManager::SendCanMessage(uint16_t msgID, uint8_t *data, uint8_t lengthOfData) {
  /* Array index */
  int i;

  CAN_outMsg.ext = 0;
  CAN_outMsg.id  =  msgID;

  if(lengthOfData > 8) {
    lengthOfData = 8;
  }

  /* Daten im MSB Format in Frame schreiben */
  for (i = 0; i<lengthOfData;i++)
     CAN_outMsg.buf[i] = data[(lengthOfData-1)-i];

  CAN_outMsg.len = i;

  /* Dissable timeout (0ms) */
  CAN_outMsg.timeout = 0;

  /* send CAN message */
  return CAN_Can0.write(CAN_outMsg);
}

int CommunicationManager::ReceiveCanMessage(uint32_t *id, uint8_t *buf, int n) {
  int result = CAN_Can0.read(CAN_inMsg);
  if(result) {
    // Get id
    *id = CAN_inMsg.id;
    // Copy data MSB to LSB
    for(int i=0; i<n; i++) {
      buf[n-1-i] = CAN_inMsg.buf[i];
    }
  }
  return result;
}

void CommunicationManager::InitNodes() {
  nNodes = 0;
  maxNodesUsed = 0;
  for(uint16_t i=0; i<COMMUNICATION_MAX_LIST_NODES; i++) {
    freeNodes[i] = &nodes[i];
  }
}

COMMUNICATION_listNode_t* CommunicationManager::NewNode() {
  if(nNodes < COMMUNICATION_MAX_LIST_NODES) {
    COMMUNICATION_listNode_t* result = freeNodes[nNodes++];

    /* Keep track of maximal usage.
     */
    if(nNodes > maxNodesUsed) {
      maxNodesUsed = nNodes;
    }
    if(maxNodesUsed == COMMUNICATION_MAX_LIST_NODES) {
      maxNodesUsed = 0;
    }

    return result;
  }

  return nullptr;
}

void CommunicationManager::DeleteNode(COMMUNICATION_listNode_t* node) {
  nNodes -= 1;
  freeNodes[nNodes] = node;
}

void CommunicationManager::InitList() {
  nodeList = nullptr;
}

bool CommunicationManager::ListAdd(COMMUNICATION_producer_t producer) {
  COMMUNICATION_listNode_t* newNode = NewNode();

  if(newNode) {
    newNode->left = nullptr;
    newNode->right = nullptr;
    newNode->producer = producer;

    ListAdd(newNode);

    /* Success */
    return true;
  }

  /* Failed: List full */
  return false;
}

void CommunicationManager::ListAdd(COMMUNICATION_listNode_t* newNode) {
  if(ListEmpty()) {
    nodeList = newNode;
    return;
  }

  if(newNode->producer.canId > nodeList->producer.canId) {
    /* Go right */
    COMMUNICATION_listNode_t* currentNode = nodeList;
    do {
      COMMUNICATION_listNode_t* rightNode = currentNode->right;

      if(nullptr == rightNode) {
        /* New elememt has the highest id in the complete list */
        currentNode->right = newNode;
        newNode->left = currentNode;
        return;
      }
      currentNode = rightNode;

    } while(newNode->producer.canId > currentNode->producer.canId);

    /* New node must be pasted inbetween existing nodes */
    COMMUNICATION_listNode_t* oldLeftNode = currentNode->left;
    currentNode->left = newNode;
    oldLeftNode->right = newNode;
    newNode->right = currentNode;
    newNode->left = oldLeftNode;

  } else {
    /* Go left: Since the list pointer is always refering to the
     *          element with the smallest id, the new node is just
     *          added to the left and the list pointer is set to the new
     *          Node.
     */
     nodeList->left = newNode;
     newNode->right = nodeList;
     nodeList = newNode;
  }
}

COMMUNICATION_producer_t CommunicationManager::ListGetHead() {
  return nodeList->producer;
}

void CommunicationManager::ListRemoveHead() {
  if(!ListEmpty()) {
    COMMUNICATION_listNode_t* newHead = nodeList->right;

    // Free storage
    DeleteNode(nodeList);

    nodeList = newHead;
  }
}

bool CommunicationManager::ListEmpty() {
  return (nullptr == nodeList);
}
