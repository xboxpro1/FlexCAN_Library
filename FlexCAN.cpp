// -------------------------------------------------------------
// a simple Arduino Teensy3.1 CAN driver
// by teachop, modified by xboxpro1
//
#include "FlexCAN.h"
#include "kinetis_flexcan.h"

static const int txb = 8; // with default settings, all buffers before this are consumed by the FIFO
static const int txBuffers = 8;
static const int rxb = 0;

// -------------------------------------------------------------
FlexCAN::FlexCAN(void)
{
  // set up the pins, 3=PTA12=CAN0_TX, 4=PTA13=CAN0_RX
  CORE_PIN3_CONFIG = PORT_PCR_MUX(2);
  CORE_PIN4_CONFIG = PORT_PCR_MUX(2);// | PORT_PCR_PE | PORT_PCR_PS;
  // select clock source 16MHz xtal
  OSC0_CR |= OSC_ERCLKEN;
  SIM_SCGC6 |=  SIM_SCGC6_FLEXCAN0;
  FLEXCAN0_CTRL1 &= ~FLEXCAN_CTRL_CLK_SRC;

  // enable CAN
  FLEXCAN0_MCR |=  FLEXCAN_MCR_FRZ;
  FLEXCAN0_MCR &= ~FLEXCAN_MCR_MDIS;
  while(FLEXCAN0_MCR & FLEXCAN_MCR_LPM_ACK);
  
  // Default mask is allow everything
  defaultMask.rtr = 0;
  defaultMask.ext = 0;
  defaultMask.id = 0;
}

// -------------------------------------------------------------
void FlexCAN::end(void)
{
  // enter freeze mode
  FLEXCAN0_MCR |= (FLEXCAN_MCR_HALT);
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK));
}

void FlexCAN::reset(void)
{
  // soft reset
  FLEXCAN0_MCR ^=  FLEXCAN_MCR_SOFT_RST;
  while(FLEXCAN0_MCR & FLEXCAN_MCR_SOFT_RST);
  
  // wait for freeze ack
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK));
  
  // disable self-reception
  FLEXCAN0_MCR |= FLEXCAN_MCR_SRX_DIS;

  //enable RX FIFO
  FLEXCAN0_MCR |= FLEXCAN_MCR_FEN;
  
  //enable TIMER SYNC
  FLEXCAN0_CTRL1 |= FLEXCAN_CTRL_TSYNC;
  
  //enable Remote Request Storing
  FLEXCAN0_CTRL2 |= FLEXCAN_CTRL2_RRS;

} 
  

// -------------------------------------------------------------
void FlexCAN::begin(uint8_t _baud)
{
  // soft reset can bus
  reset();
  
  // segment splits and clock divisor based on baud rate
  switch (_baud){
	  
	case (CAN_5KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(199)); // 5000 baud Prescaler -> 199  Tq -> 16
    break;
	
	case (CAN_10KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(99));	// 10000 baud Prescaler -> 99  Tq -> 16
	break;	
    
	case (CAN_20KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(49));	// 20000 baud Prescaler -> 49  Tq -> 16
	break;	
	
	case (CAN_25KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(39));	// 25000 baud Prescaler -> 39  Tq -> 16
	break;	
	
	case (CAN_31K25BPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(31));	// 31250 baud Prescaler -> 31  Tq -> 16
	break;	
	
	case (CAN_33KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(29));  // 33333 baud Prescaler -> 29  Tq -> 16
    break;
	
	case (CAN_40KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(24));	// 40000 baud Prescaler -> 24  Tq -> 16
	break;
    
    case (CAN_50KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(19));  // 50000 baud Prescaler -> 19  Tq -> 16
    break;
	
	case (CAN_80KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(3) | FLEXCAN_CTRL_PSEG2(1) | FLEXCAN_CTRL_PRESDIV(19));  // 80000 baud Prescaler -> 19 !!Tq -> 10!!
    break;
	
	case (CAN_83K3BPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(11));  // 83333 baud Prescaler -> 11  Tq -> 16
    break;
	
	case (CAN_95KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(2)
                      | FLEXCAN_CTRL_PSEG1(4) | FLEXCAN_CTRL_PSEG2(2) | FLEXCAN_CTRL_PRESDIV(13));  // 95000 baud Prescaler -> 13 !!Tq -> 12!!
    break;
    
    case (CAN_100KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(9));  // 100000 baud Prescaler -> 9  Tq -> 16
    break;
    
    case (CAN_125KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(7));  // 125000 baud Prescaler -> 7  Tq -> 16
    break;
	
	case (CAN_200KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(4));  // 200000 baud Prescaler -> 4  Tq -> 16
    break;
    
    case (CAN_250KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(3));  // 250000 baud Prescaler -> 3  Tq -> 16
    break;
    
    case (CAN_500KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(1));  // 500000 baud Prescaler -> 1  Tq -> 16
    break;
	
	case (CAN_666KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(2)
                      | FLEXCAN_CTRL_PSEG1(4) | FLEXCAN_CTRL_PSEG2(2) | FLEXCAN_CTRL_PRESDIV(1));  // 666666 baud Prescaler -> 1 !!Tq -> 12!!
    break;
    
    case (CAN_1000KBPS):
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(3)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(0));  // 100000 baud Prescaler -> 0  Tq -> 16
    break;
	
	default:    // 125000
        FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(2)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(7));  // 125000 baud Prescaler -> 7  Tq -> 16
    break;
  }
  
  //enable reception of all messages
  FLEXCAN0_RXMGMASK = 0;
  FLEXCAN0_RXFGMASK = ((defaultMask.rtr?1:0) << 31) | ((defaultMask.ext?1:0) << 30) | (FLEXCAN_MB_ID_IDSTD(defaultMask.id) << 1);
   
  // start the CAN
  FLEXCAN0_MCR &= ~(FLEXCAN_MCR_HALT);
  // wait till exit of freeze mode
  while(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK);

  // wait till ready
  while(FLEXCAN0_MCR & FLEXCAN_MCR_NOT_RDY);

  //set tx buffers to inactive
  for (int i = txb; i < txb + txBuffers; i++) {
    FLEXCAN0_MBn_CS(i) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  } 
}

// -------------------------------------------------------------
int FlexCAN::connect(int _speed, int _check)	// auto connect with speed test, speed index 1 - 18 , Error ---> 255
{
  int _speedindex = 255;
   
  if (_speed == 0){							// if _speed = 0 --> check the bus speed from 1 to 18
    for (int _x = 1; _x <= 18; _x ++){
      begin(_x);
      delay(100);
      if(1 == available()){					// if frame available speed is correct
        _speedindex = _x;
        break;
      }
    }
  }
  else {									// if _speed from 1 to 18 use given bus speed
    begin(_speed);
    if(_check == 1){						// if _check = 1 check the given bus speed
      delay(100);
      if(1 == available()){					// if frame available speed is correct
        _speedindex = _speed;
      }
    }
    else {
      _speedindex = _speed;					// return the given bus speed index without speed test
    }
  }
  if(_speedindex == 255){					// enter freeze mode if speed is incorrect
	  end();
  }
  return _speedindex;						// return the bus speed index 1 - 18 , Error ---> 255
}

// -------------------------------------------------------------
void FlexCAN::setMask(const CAN_filter_t &mask)
{
  // enter freeze mode
  FLEXCAN0_MCR |= (FLEXCAN_MCR_HALT);
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK));
  
  FLEXCAN0_RXMGMASK = 0;

  //enable reception of all messages that fit the mask
  if (mask.ext) {
    FLEXCAN0_RXFGMASK = ((mask.rtr?1:0) << 31) | ((mask.ext?1:0) << 30) | ((mask.id & FLEXCAN_MB_ID_EXT_MASK) << 1);
  } else {
    FLEXCAN0_RXFGMASK = ((mask.rtr?1:0) << 31) | ((mask.ext?1:0) << 30) | (FLEXCAN_MB_ID_IDSTD(mask.id) << 1);
  }

  // start the CAN
  FLEXCAN0_MCR &= ~(FLEXCAN_MCR_HALT);
  // wait till exit of freeze mode
  while(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK);

  // wait till ready
  while(FLEXCAN0_MCR & FLEXCAN_MCR_NOT_RDY);

  //set tx buffers to inactive
  for (int i = txb; i < txb + txBuffers; i++) {
    FLEXCAN0_MBn_CS(i) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  }
  
}


// -------------------------------------------------------------
void FlexCAN::setFilter(const CAN_filter_t &filter, uint8_t n)
{
  if ( n < 8) {
    if (filter.ext) {
      FLEXCAN0_IDFLT_TAB(n) = ((filter.rtr?1:0) << 31) | ((filter.ext?1:0) << 30) | ((filter.id & FLEXCAN_MB_ID_EXT_MASK) << 1);
    } else {
      FLEXCAN0_IDFLT_TAB(n) = ((filter.rtr?1:0) << 31) | ((filter.ext?1:0) << 30) | (FLEXCAN_MB_ID_IDSTD(filter.id) << 1);
    }
  }
}

// -------------------------------------------------------------
void FlexCAN::clearFilter(void)
{
  	for (int _x = 0; _x < 8; _x ++){
		setFilter(defaultMask, _x);
	}
}

// -------------------------------------------------------------
void FlexCAN::clearMask(void)
{
	setMask(defaultMask);
}
	
// -------------------------------------------------------------
int FlexCAN::available(void)
{
  //In FIFO mode, the following interrupt flag signals availability of a frame
  return (FLEXCAN0_IFLAG1 & FLEXCAN_IMASK1_BUF5M)? 1:0;
}

int FlexCAN::synchron(void)
{
  //Check if synchronized with the can bus
  return (FLEXCAN0_ESR1 & FLEXCAN_ESR_SYNCH)? 1:0;
}


// -------------------------------------------------------------
int FlexCAN::read(CAN_message_t &msg)
{
  unsigned long int startMillis;

  startMillis = msg.timeout? millis() : 0;

  while( !available() ) {
    if ( !msg.timeout || (msg.timeout<=(millis()-startMillis)) ) {
      // early EXIT nothing here
      return 0;
    }
    yield();
  }

  // get identifier and dlc
  msg.len = FLEXCAN_get_length(FLEXCAN0_MBn_CS(rxb));
  msg.req = (FLEXCAN0_MBn_CS(rxb) & FLEXCAN_MB_CS_RTR)? 1:0;
  msg.ext = (FLEXCAN0_MBn_CS(rxb) & FLEXCAN_MB_CS_IDE)? 1:0;
  msg.id  = (FLEXCAN0_MBn_ID(rxb) & FLEXCAN_MB_ID_EXT_MASK);
  
  if(!msg.ext) {
    msg.id >>= FLEXCAN_MB_ID_STD_BIT_NO;
  }

  // copy out message
  uint32_t dataIn = FLEXCAN0_MBn_WORD0(rxb);
  msg.buf[3] = dataIn;
  dataIn >>=8;
  msg.buf[2] = dataIn;
  dataIn >>=8;
  msg.buf[1] = dataIn;
  dataIn >>=8;
  msg.buf[0] = dataIn;
  if ( 4 < msg.len ) {
    dataIn = FLEXCAN0_MBn_WORD1(rxb);
    msg.buf[7] = dataIn;
    dataIn >>=8;
    msg.buf[6] = dataIn;
    dataIn >>=8;
    msg.buf[5] = dataIn;
    dataIn >>=8;
    msg.buf[4] = dataIn;
  }
  for( int loop=msg.len; loop<8; ++loop ) {
    msg.buf[loop] = 0;
  }
  
  //notify FIFO that message has been read
  msg.timestamp  = (FLEXCAN0_MBn_CS(rxb) & FLEXCAN_MB_CS_TIMESTAMP_MASK);
  FLEXCAN0_IFLAG1 = FLEXCAN_IMASK1_BUF5M;

  return 1;
}


// -------------------------------------------------------------
int FlexCAN::write(const CAN_message_t &msg)
{
  unsigned long int startMillis;

  startMillis = msg.timeout? millis() : 0;

  // find an available buffer
  int buffer = -1;
  for ( int index = txb; ; ) {
    if ((FLEXCAN0_MBn_CS(index) & FLEXCAN_MB_CS_CODE_MASK) == FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE)) {
      buffer = index;
      break;// found one
    }
    if ( !msg.timeout ) {
      if ( ++index >= (txb+txBuffers) ) {
        return 0;// early EXIT no buffers available
      }
    } else {
      // blocking mode, only 1 txb used to guarantee frames in order
      if ( msg.timeout <= (millis()-startMillis) ) {
        return 0;// timed out
      }
      yield();
    }
  }

  // transmit the frame
  FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  if(msg.ext) {
    FLEXCAN0_MBn_ID(buffer) = (msg.id & FLEXCAN_MB_ID_EXT_MASK);
  } else {
    FLEXCAN0_MBn_ID(buffer) = FLEXCAN_MB_ID_IDSTD(msg.id);
  }
  FLEXCAN0_MBn_WORD0(buffer) = (msg.buf[0]<<24)|(msg.buf[1]<<16)|(msg.buf[2]<<8)|msg.buf[3];
  FLEXCAN0_MBn_WORD1(buffer) = (msg.buf[4]<<24)|(msg.buf[5]<<16)|(msg.buf[6]<<8)|msg.buf[7];
  if(msg.ext) {
    if(msg.req){
      FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE | FLEXCAN_MB_CS_RTR;
    } else {
      FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE;
      }
  } else {
    if(msg.req){
      FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_RTR;
    } else {
      FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len);
    }
  }

  return 1;
}