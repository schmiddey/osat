#include "Arduino.h"


#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
// #include <MQTTClient.h>
#include <WiFiUdp.h>

struct OsatConfig{
  uint8_t    id         = 255;
  uint16_t   hit_thresh = 50;
  uint16_t   timeout_ms = 1500;

  String toString() const
  {
    String str("OsatConfig #id:");
    str += String(id);
    str += String(" #hit_thresh:");
    str += String(hit_thresh);
    str += String(" #timeout_ms:");
    str += String(timeout_ms);
    return str;
  }
};

// https://stackoverflow.com/questions/51731313/cross-platform-crc8-function-c-and-python-parity-check
uint8_t crc8( uint8_t *addr, uint8_t len) {
      uint8_t crc=0;
      for (uint8_t i=0; i<len;i++) {
         uint8_t inbyte = addr[i];
         for (uint8_t j=0;j<8;j++) {
             uint8_t mix = (crc ^ inbyte) & 0x01;
             crc >>= 1;
             if (mix) 
                crc ^= 0x8C;
         inbyte >>= 1;
      }
    }
   return crc;
}

namespace cfg{
  enum Config{
    ID           = 0,
    THRESH_LOW,
    THRESH_HIGH,
    TIMEOUT_LOW,
    TIMEOUT_HIGH,
    CRC,
    CFG_SIZE
  };
}



void write_osat_config(const OsatConfig& cfg)
{
  EEPROM.begin(512);

  uint8_t eeprom_data[cfg::CFG_SIZE];

  eeprom_data[cfg::ID]            = cfg.id;
  eeprom_data[cfg::THRESH_LOW]    = lowByte(cfg.hit_thresh);
  eeprom_data[cfg::THRESH_HIGH]   = highByte(cfg.hit_thresh);
  eeprom_data[cfg::TIMEOUT_LOW]   = lowByte(cfg.timeout_ms);
  eeprom_data[cfg::TIMEOUT_HIGH]  = highByte(cfg.timeout_ms);
  eeprom_data[cfg::CRC]           = crc8(eeprom_data, cfg::CFG_SIZE - 1);
  
  EEPROM.write(cfg::ID,            eeprom_data[cfg::ID]);
  EEPROM.write(cfg::THRESH_LOW,    eeprom_data[cfg::THRESH_LOW]);
  EEPROM.write(cfg::THRESH_HIGH,   eeprom_data[cfg::THRESH_HIGH]);
  EEPROM.write(cfg::TIMEOUT_LOW,   eeprom_data[cfg::TIMEOUT_LOW]);
  EEPROM.write(cfg::TIMEOUT_HIGH,  eeprom_data[cfg::TIMEOUT_HIGH]);
  EEPROM.write(cfg::CRC,           eeprom_data[cfg::CRC]);
  EEPROM.end();
}

OsatConfig read_osat_config()
{
  OsatConfig cfg;

  EEPROM.begin(512);

  uint8_t id            = EEPROM.read(cfg::ID);
  uint8_t thresh_low    = EEPROM.read(cfg::THRESH_LOW);
  uint8_t thresh_high   = EEPROM.read(cfg::THRESH_HIGH);
  uint8_t timeout_low   = EEPROM.read(cfg::TIMEOUT_LOW);
  uint8_t timeout_high  = EEPROM.read(cfg::TIMEOUT_HIGH);

  EEPROM.end();

  cfg.id = id;
  cfg.hit_thresh = word(thresh_high, thresh_low);
  cfg.timeout_ms = word(timeout_high, timeout_low);

  return cfg;
}

/**
 * @brief checks if eeprom_cfg is valid, if not, init with default
 * 
 * @return true is eeprom config is ok, false if not but now valid (with default values)
 */
bool check_osat_config()
{
  uint8_t eeprom_data[cfg::CFG_SIZE];
  
  EEPROM.begin(512);

  eeprom_data[cfg::ID]           = EEPROM.read(cfg::ID);
  eeprom_data[cfg::THRESH_LOW]   = EEPROM.read(cfg::THRESH_LOW);
  eeprom_data[cfg::THRESH_HIGH]  = EEPROM.read(cfg::THRESH_HIGH);
  eeprom_data[cfg::TIMEOUT_LOW]  = EEPROM.read(cfg::TIMEOUT_LOW);
  eeprom_data[cfg::TIMEOUT_HIGH] = EEPROM.read(cfg::TIMEOUT_HIGH);  
  eeprom_data[cfg::CRC]          = EEPROM.read(cfg::CRC);

  EEPROM.end();

  uint8_t tmp_crc = crc8(eeprom_data, cfg::CFG_SIZE - 1);

  if(eeprom_data[cfg::ID] == 0 || tmp_crc != eeprom_data[cfg::CRC])
  {// EEPROM data not valid!!!
    OsatConfig cfg;
    write_osat_config(cfg);
    return false;
  }
  return true;
}



/**
 * @todo ESP.restart(); if needed somewhere :)
 * 
 */

struct Color{
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  Color(const uint8_t _r, const uint8_t _g, const uint8_t _b) :
    r(_r), g(_g), b(_b)
  {}
  Color() = default;
};

/**
 * @note  I know its overloaded to implement a class here. But I kind a need it :)
 */
class Led{
public:
  Led()
  {
     _pixel = Adafruit_NeoPixel(1, D4, NEO_GRB + NEO_KHZ800);

     _colors[OFF]     = Color(0,0,0);
     _colors[RED]     = Color(255,0,0);
     _colors[ORANGE]  = Color(255,107,0);
     _colors[GREEN]   = Color(0,255,0);
     _colors[BLUE]    = Color(0,0,255);
     _colors[VIOLETT] = Color(255,0,255);
     _colors[WHITE]  = Color(255,255,255);
  }
  ~Led()
  {}
  
  enum ColorEnum{
    OFF = 0,
    RED,
    ORANGE,
    GREEN,
    BLUE,
    VIOLETT,
    WHITE,
    SIZE
  };

  void init()
  {
    _pixel.begin();
    _pixel.clear();
    this->setColor(OFF);
  }

  /**
   * @brief Set the Color object
   * 
   * @todo add intensity ...
   * 
   * @param color 
   * @param intensity 
   */
  void setColor(const ColorEnum color, const double intensity = 1.0)
  {
    _current_color = color;
    
    double tmp_intens = intensity;
    constrain(tmp_intens, 0.0, 1.0);

    _current_intens = intensity;

    Color c = _colors[color];

    c.r = round(static_cast<double>(c.r) * tmp_intens);
    c.g = round(static_cast<double>(c.g) * tmp_intens);
    c.b = round(static_cast<double>(c.b) * tmp_intens);

    _pixel.setPixelColor(0, _pixel.Color(c.r, c.g, c.b));
    _pixel.show();

  }

  void blink(const ColorEnum color, unsigned long delay_ms = 2, const double intensity = 1.0)
  {
    this->setColor(color, intensity);
    delay(delay_ms); //todo check perfect time :D
    this->setColor(_current_color, _current_intens);
  }

  // void tick()
  // {
  //   _pixel.show();
  // }

private:
  Adafruit_NeoPixel _pixel;

  ColorEnum    _current_color  = Led::OFF;
  double       _current_intens = 1.0;

  Color _colors[SIZE];
};


Led g_led;



/**
 * @brief Communication class for osat server
 * 
 * Comm protocoll for receiving... (All cmds are sended as string...)
 * Commands:
 * "CMD-ACTIVE"    -> Activates OSAT -> waiting for hit... no timeout
 * "CMD-ACTIVE_TO" -> Activates Osat -> waiting for hit, but sets it inactive after timout (from cfg)
 * "CMD-INACTIVE"  -> Deactivates OSAT
 * "CMD-FLASH"     -> Activates Flash Mode -> Flashes when hit
 * 
 * Configs: (All configs are permanently saved in EEPROM)
 * "CFG-ID-xxx"       -> Sets ID of this Module (1..255) 255 default
 * "CFG-THRESH-xxx"   -> Sets hit-threshold for this Module
 * "CFG-TIMEOUT-xxx"  -> Sets timeout in [ms] for ACTIVE_TO mode
 * "CFG-REQUEST"      -> Sends current cfg
 * 
 * @todo add cmd for sending config!!
 * 
 */
class OsatComm{
public:

  enum State {
    IDLE = 0,
    ACTIVE,
    ACTIVE_TO,
    INACTIVE,
    FLASH
  };

  OsatComm(WiFiServer& server) : 
    _server(server)
  { }
  
  ~OsatComm() { }
  
  void connect()
  {
    delay(100);
    // this->transmit("init... check Config...");

    //handle EEPPROM Config
    check_osat_config();

    _osat_config = read_osat_config();
    this->transmit(_osat_config.toString());

  }

  void tickTcpIn()
  {
    if(_server.hasClient())
    {
      _serverClient = _server.available();
      // g_led.blink(Led::VIOLETT, 500);

      // Serial.write("got client");
    }
    int buff_cnt = 0;
    while(true)
    {
      // int mil = millis();
      int av = _serverClient.available();
      // Serial.println(millis() - mil);
      if(!av)
      {
        break;
      }
      _buffer[buff_cnt++] = _serverClient.read();
      // Serial.write(serverClient.read());
    }
    if(buff_cnt)
    {
      _buffer[buff_cnt] = 0; //for safety close string;
      //got cmd.... parse it;
      this->parseCmd(_buffer);
    }
  }

  void sendHit()
  {
    this->transmit("hit\n");
  }

  void sendTimeout()
  {
    this->transmit("timeout");
  }

  void transmit(const String& str) const
  {
    String sendstr(str); //copy
    sendstr = String(_osat_config.id) + String("|") + sendstr;
    //fix endline
    if(!sendstr.endsWith("\n"))
    {
      sendstr += '\n';
    }

    {
      WiFiClient cl;
      while(!cl.connect(_SERVER, _PORT))
      {
        optimistic_yield(100);
      }
      cl.setNoDelay(true);
      cl.setSync(false);
      cl.disableKeepAlive();
      delay(5);
      // optimistic_yield(10000);
      cl.print(sendstr.c_str());
      // cl.flush(100);
      // optimistic_yield(10000);
    }
    delay(5);
  }

  State getState() const
  {
    return _state;
  }

  void setState(const State state)
  {
    _state = state;
  }

  const OsatConfig& getConfig() const
  {
    return _osat_config;
  }

  bool checkHit()
  {
    int piezo = analogRead(A0);
    delay(9);  //very important here -> if not there wifi will disconnect ... sadly :(
  
    if(piezo > _osat_config.hit_thresh)
    {
      return true;
    }
    return false;
  }

private: //functions

  void parseCmd(const char* data)
  {
    String msg(data);
    String msg_type    = msg.substring(0,3);
    String msg_contend = msg.substring(4);

    // this->transmit(msg_type.c_str());
    // this->transmit(msg_contend.c_str());
    
    //remove \n from msg_contend
    if(msg_contend.endsWith("\n"))
    {
      //remove \n
      msg_contend.remove(msg_contend.length() -1);
    }

    bool cmd_ok = false;
    bool cfg_ok = false;

    //decide if cmd or cfg... 
    if(msg_type == String("CMD"))
    {
      if(msg_contend == String("ACTIVE"))
      {
        this->setState(ACTIVE);
        cmd_ok = true;
      }
      else if(msg_contend == String("ACTIVE_TO"))
      {
        this->setState(ACTIVE_TO);
        cmd_ok = true;
      }
      else if(msg_contend == String("INACTIVE"))
      {
        this->setState(INACTIVE);
        cmd_ok = true;
      }
      else if(msg_contend == String("FLASH"))
      {
        this->setState(FLASH);
        cmd_ok = true;
      }
      //invalid
    }
    else if(msg_type == String("CFG"))
    {
      //CFG-ID-xxx
      //CFG-THRESH-xxx
      //CFG-TIMEOUT-xxx
      if(msg_contend.substring(0,2) == String("ID"))
      {
        //set id
        unsigned int tmp_id = msg_contend.substring(3).toInt();
        if(tmp_id && tmp_id <=255)
        {  
          _osat_config.id = static_cast<uint8_t>(tmp_id);
          write_osat_config(_osat_config);
          cfg_ok = true;
        }
      }
      else if(msg_contend.substring(0,6) == String("THRESH"))
      {
        unsigned int tmp_thresh = msg_contend.substring(7).toInt();
        if(tmp_thresh &&tmp_thresh <= 1023)
        {  
          _osat_config.hit_thresh = tmp_thresh;
          write_osat_config(_osat_config);
          cfg_ok = true;
        }
      }
      else if(msg_contend.substring(0,7) == String("TIMEOUT"))
      {
        unsigned int tmp_timeout = msg_contend.substring(8).toInt();
        if(tmp_timeout && tmp_timeout <=10000)
        {  
          _osat_config.timeout_ms = tmp_timeout;
          write_osat_config(_osat_config);
          cfg_ok = true;
        }
      }
      else if(msg_contend.substring(0,8) == String("REQUEST"))
      {
        //only request cfg
        cfg_ok = true;
      }
    }

    if(cmd_ok)
    {
      //send ack
      // this->transmit(msg);
    }
    else if(cfg_ok)
    {
      //send ack
      // this->transmit(msg);
      //send new config
      this->transmit(_osat_config.toString());
    }
    else  //invalid command
    {
      this->transmit("Got invalid Command ...");
    }
    //For ack  
    // this->transmit(data);
  }
  
private:
  WiFiServer& _server;        //tcp server;
  WiFiClient  _serverClient;  //only one client for server


  const char*    _SERVER = "192.168.5.1";
  // const char*    _SERVER = "10.42.0.1";
  const uint16_t _PORT   = 1337;

  char _buffer[50];

  State _state = IDLE;

  OsatConfig _osat_config;
};

WiFiServer g_wifiserver(1335);


//router @ home
const auto ssid = "osat_base";
const auto pass = "1234567890";
// const auto ssid = "schmiddeys_WWW";
// const auto pass = "1234567890";

// // think ap
// const auto ssid = "think";
// const auto pass = "NPWZtluM";

OsatComm g_osat_com(g_wifiserver);




void setup() 
{
  // bool ret_check_cfg = check_osat_config();

  // g_osat_config = read_osat_config();

  WiFi.begin(ssid, pass);

  g_led.init();

  g_led.setColor(Led::RED);

  while (WiFi.status() != WL_CONNECTED) {
    g_led.setColor(Led::BLUE);
    delay(250);
    g_led.setColor(Led::OFF);
    delay(250);
  }


  g_wifiserver.begin();
  g_wifiserver.setNoDelay(true);

  // WiFi.setAutoReconnect(true);
  g_led.setColor(Led::GREEN);
  delay(1000);

  //osat com
  g_osat_com.connect();

  g_led.setColor(Led::VIOLETT);
  delay(1000);
}

int cnt = 0;

OsatComm::State old_state = OsatComm::IDLE;

uint32_t timeout = 0;

void loop() 
{
  g_osat_com.tickTcpIn();

  switch(g_osat_com.getState()) {
    case OsatComm::ACTIVE:
    {
      //detect state change flag
      if(old_state != OsatComm::ACTIVE)
      {
        g_led.setColor(Led::ORANGE);
      }
      
      old_state = OsatComm::ACTIVE;

      if(g_osat_com.checkHit())
      {//got hit
        //LED off
        g_led.setColor(Led::OFF);
        //send hit
        g_osat_com.sendHit();

        //set inactive
        g_osat_com.setState(OsatComm::INACTIVE);
      } 
      break;
    }
    case OsatComm::ACTIVE_TO:
    {
      //detect state change flag
      if(old_state != OsatComm::ACTIVE_TO)
      {
        g_led.setColor(Led::RED);
        //start timer
        timeout = millis();
      }
      
      old_state = OsatComm::ACTIVE_TO;

      //check timeout
      if((millis() - timeout) > g_osat_com.getConfig().timeout_ms)
      {
        //got timeout
        g_led.setColor(Led::OFF);

        g_osat_com.sendTimeout();

        g_osat_com.setState(OsatComm::INACTIVE);
        break;
      }

      if(g_osat_com.checkHit())
      {//got hit
        //LED off
        g_led.setColor(Led::OFF);
        //send hit
        g_osat_com.sendHit();

        //set inactive
        g_osat_com.setState(OsatComm::INACTIVE);
      } 
      break;
    }
    case OsatComm::INACTIVE:
    {
      if(old_state != OsatComm::INACTIVE)
      {

        g_led.setColor(Led::OFF);
      }
      //do nothing
      old_state = OsatComm::INACTIVE;
      delay(5);
      break;
    }
    case OsatComm::FLASH:
    {
      if(old_state != OsatComm::FLASH)
      {
        //led WHITE
        g_led.setColor(Led::WHITE, 0.1);
      }

      old_state = OsatComm::FLASH;
      
      
      if(g_osat_com.checkHit())
      {
        g_osat_com.sendHit();
        g_led.setColor(Led::GREEN);
        delay(50);
        g_led.setColor(Led::OFF);
        delay(100);
        g_led.setColor(Led::WHITE, 0.1);
      }
      break;
    }
    default: //idle
    {
      if(cnt%100 == 0)
      {
        g_led.setColor(Led::VIOLETT);
        delay(10);
        g_led.setColor(Led::OFF);
        // delay(50);
        //send config
        g_osat_com.transmit(g_osat_com.getConfig().toString());
        delay(10);
        g_osat_com.transmit(String(WiFi.RSSI()));
        

      }
      delay(10);
      break;
    }
  }
 
 
  ++cnt;
}