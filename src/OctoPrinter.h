#ifndef OctoPrinter_h
#define OctoPrinter_h

#include "Arduino.h"
#include "WiFi.h"
#include "IPAddress.h"
#include "ArduinoJson.h"

struct temperatureTracker {
  double actual;
  int target;
  int offset;
};

struct jobTimer {
  int raw;
  int days;
  int hours;
  int minutes;
  int seconds;
  String formatted;
};

class OctoPrinter {
public:
  OctoPrinter(String key, String host, int port);
  void update();
  void begin();

  /* 
    These functions are used to check on the printer's status flags.
    All they do is return the state of the boolean flag.
  */
  bool operational();
  bool paused();
  bool printing();
  bool cancelling();
  bool pausing();
  bool error();
  bool ready();
  bool closed();
  bool closedOrError();

  //give job related information
  String remainingFormatted();
  String fileName();
  double progress();

  /*
    Functions to control jobs.
    These each return the HTTP status code from the server. As OctoPrint's
    API is incapable of waiting for the printer response without holding
    the entire webserver, we won't get more information than that.
  */
  int startJob();
  int cancelJob();
  int restartJob();
  int pauseJob();
  int resumeJob();
  int toggleJobPauseState();

  //give server related information
  String serverVersion();
  String apiVersion();

  //temperature related functions
  double toolActual();
  int toolTarget();
  int toolOffset();
  double bedActual();
  int bedTarget();
  int bedOffset();
  double chamberActual();
  int chamberTarget();
  int chamberOffset();

private:
  WiFiClient _client;
  void _setTime(int, int);
  String _requester(String);
  String _poster(String, String);
  void _parsePrinter(String);
  void _parseSystem(String);
  void _parseJob(String);
  String _parseConnection(String);
  void _parseProfile(String);

  String _apiKey;
  IPAddress _host;
  String _hostAddress;
  int _port;
  bool _doOnce;

  struct {
    String _apiVersion;
    String _serverVersion;
  } _server;
  struct {
    jobTimer _elapsed;
    jobTimer _remaining;
    const char* _fileName;
    double _progress;
    double _rawProgress;
  } _job;
  struct {
    bool _operational;
    bool _paused;
    bool _printing;
    bool _cancelling;
    bool _pausing;
    bool _error;
    bool _ready;
    bool _closed;
    bool _closedOrError;
  } _is;
  struct {
    bool _heatedBed;
    bool _heatedChamber;
    int _toolCout;
  } _has;
  temperatureTracker _bed;
  temperatureTracker _chamber;
  temperatureTracker _tool;
};

#endif