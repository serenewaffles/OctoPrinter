#include "Arduino.h"
#include "OctoPrinter.h"
#include "WiFi.h"
#include "ArduinoJson.h"

OctoPrinter::OctoPrinter(String apiKey, String hostAddress, int port) {
    _apiKey = apiKey;
    _hostAddress = hostAddress;
    _port = port;
    _host.fromString(hostAddress);
    _doOnce = false;
}

void OctoPrinter::begin() {
    if (!_doOnce) {
        _parseSystem(_requester("/api/version"));
        update();
        _doOnce = true;
    }
}

void OctoPrinter::update() {
    _parseConnection(_requester("/api/connection"));
    // if (!_is._closed) {
        _parsePrinter(_requester("/api/printer?exclude=sd"));
        if (_is._printing) {
            _parseJob(_requester("/api/job"));
        }
    // }
}

/* 
  The following functions return printer states.
*/
bool OctoPrinter::operational() {
    return _is._operational;
}

bool OctoPrinter::paused() {
    return _is._paused;
}

bool OctoPrinter::printing() {
    return _is._printing;
}

bool OctoPrinter::cancelling() {
    return _is._cancelling;
}

bool OctoPrinter::pausing() {
    return _is._paused;
}

bool OctoPrinter::error() {
    return _is._error;
}

bool OctoPrinter::ready() {
    return _is._ready;
}

bool OctoPrinter::closedOrError() {
    return _is._closedOrError;
}

String OctoPrinter::remainingFormatted() {
    return _job._remaining.formatted;
}

String OctoPrinter::fileName() {
    return _job._fileName;
}

// These functions are used to get the tool and bed temperature stats.

double OctoPrinter::toolActual() {
    return _tool.actual;
}

int OctoPrinter::toolTarget() {
    return _tool.target;
}

int OctoPrinter::toolOffset() {
    return _tool.offset;
}

double OctoPrinter::bedActual() {
    return _bed.actual;
}

int OctoPrinter::bedTarget() {
    return _bed.target;
}

int OctoPrinter::bedOffset() {
    return _bed.offset;
}

/*
  This handy dandy helper function takes the raw job times in seconds from OctoPrint
  and does a number of operations on them:
    1. Uses the elapsed and remaining times to calculate progress.
        Rather than trusting OctoPrint's calculated progress, we're making our own. The
        major benefit of doing it this way is accurately tracking the progress when a plugin
        (i.e. PrintTimeGenius) modifies the time estimate. OctoPrint doesn't report the
        progress based on the PTG times.
    2. Takes the raw elapsed and remaining times and breaks them down into days, hours, minutes,
       and seconds.
    3. Uses the numbers calculated in step 2 and constructs a formatted string, which can be
       fed directly into a print statement.
*/

void OctoPrinter::_setTime(int elapsed, int remaining) {
    _job._rawProgress = (double)elapsed / ((double)elapsed + (double)remaining);
    _job._progress = _job._rawProgress * 100;

    _job._elapsed.raw = elapsed;
    _job._elapsed.days = elapsed / (24 * 60 * 60);
    _job._elapsed.hours = (elapsed % (24 * 60 * 60)) / (60 * 60);
    _job._elapsed.minutes = (elapsed % (60 * 60) / 60);
    _job._elapsed.seconds = elapsed % 60;
    _job._elapsed.formatted = "";
    _job._elapsed.formatted += String(_job._elapsed.days);
    _job._elapsed.formatted += (_job._elapsed.hours < 10) ? ":0" : ":";
    _job._elapsed.formatted += String(_job._elapsed.hours);
    _job._elapsed.formatted += (_job._elapsed.minutes < 10) ? ":0" : ":";
    _job._elapsed.formatted += String(_job._elapsed.minutes);
    _job._elapsed.formatted += (_job._elapsed.seconds < 10) ? ":0" : ":";
    _job._elapsed.formatted += String(_job._elapsed.seconds);

    _job._remaining.raw = remaining;
    _job._remaining.hours = remaining / (60 * 60);
    _job._remaining.minutes = (remaining % (60 * 60) / 60);
    _job._remaining.seconds = remaining % 60;
    _job._remaining.formatted = "";
    _job._remaining.formatted += String(_job._remaining.days);
    _job._remaining.formatted += (_job._remaining.hours < 10) ? ":0" : ":";
    _job._remaining.formatted += String(_job._remaining.hours);
    _job._remaining.formatted += (_job._remaining.minutes < 10) ? ":0" : ":";
    _job._remaining.formatted += String(_job._remaining.minutes);
    _job._remaining.formatted += (_job._remaining.seconds < 10) ? ":0" : ":";
    _job._remaining.formatted += String(_job._remaining.seconds);
}

String OctoPrinter::_requester(String uri) {
    if (!_client.connect(_host, _port)) {
        return "";
    } else {
        _client.println("GET " + uri + " HTTP/1.1");
        _client.println("Host: " + _hostAddress);
        _client.println("Cache-Control: no-cache");
        _client.println("X-Api-Key: " + _apiKey);
        _client.println("");

        while (_client.connected()) {
            String line = _client.readStringUntil('\n');
            // Serial.println(line);
            if (line == "\r") {
                break;
            }
        }
    
        String response;
        while (_client.available()) {
            response = _client.readString();
        }
        _client.stop();
        // Serial.println(response);
        return response;
    }
}

String OctoPrinter::_poster(String uri, String body) {
    if (!_client.connect(_host, _port)) {
        return "ERROR";
    } else {
        _client.println("POST " + uri + " HTTP/1.1");
        _client.println("Host: " + _hostAddress);
        _client.println("Cache-Control: no-cache");
        _client.println("X-Api-Key: " + _apiKey);
        _client.println("Content-Type: application/json");
        _client.print("Content-Length: ");
        _client.println(body.length());
        _client.println("");
        _client.println(body);
        _client.println("");

        String response;
        while (_client.connected()) {
            String line = _client.readStringUntil('\n');
            if (line.startsWith("HTTP")) {
                response = line.substring(9, 12);
            }
            if (line == "\r") {
            break;
            }
        }

        _client.stop();
        return response;
    }
}

void OctoPrinter::_parsePrinter(String json) {
    StaticJsonDocument<512> doc;
    char* json_c_str = &json[0u];
    DeserializationError error = deserializeJson(doc, json_c_str, json.length());

    if (error) {
        return;
    }
    JsonObject temperature = doc["temperature"];

    JsonObject temperature_tool0 = temperature["tool0"];
    _tool.actual = temperature_tool0["actual"];
    _tool.target = temperature_tool0["target"];
    
    JsonObject temperature_bed = temperature["bed"];
    _bed.actual = temperature_bed["actual"];
    _bed.target = temperature_bed["target"];

    JsonObject state_flags = doc["state"]["flags"];
    _is._operational = state_flags["operational"];
    _is._paused = state_flags["paused"];
    _is._printing = state_flags["printing"];
    _is._cancelling = state_flags["cancelling"];
    _is._pausing = state_flags["pausing"];
    _is._error = state_flags["error"];
    _is._ready = state_flags["ready"];
    _is._closedOrError = state_flags["closedOrError"];
}

void OctoPrinter::_parseSystem(String json) {
    char* input = &json[0u];
    StaticJsonDocument<48> doc;
    DeserializationError error = deserializeJson(doc, input, json.length());

    if (error) {
        return;
    }
    
    const char* serverAPIVersion = doc["api"];  // "0.1"
    const char* serverVersion = doc["server"];

    _server._serverVersion = String(serverVersion);
    _server._apiVersion = String(serverAPIVersion);
}

void OctoPrinter::_parseJob(String json) {
    StaticJsonDocument<384> doc;
    char* input = &json[0u];
    DeserializationError error = deserializeJson(doc, input, json.length());
    
    if (error) {
        return;
    }

    JsonObject progress = doc["progress"];
    _setTime(progress["printTime"], progress["printTimeLeft"]);
    
    JsonObject file = doc["file"];
    _job._fileName = file["name"];
}

String OctoPrinter::_parseConnection(String json) {
    char* input = &json[0u];
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, input, json.length());
    
    if (error) {
        return "";
    }
    const char* currentPrinterProfile = doc["printerProfile"];
    JsonObject current = doc["current"];
    const char* currentState = current["state"];
    if (String(currentState) != "Closed") {
        _is._closed = false;
    } else {
        _is._closed = true;
    }
    String response = String(currentPrinterProfile);
    return response;
}

String OctoPrinter::serverVersion() {
    return _server._serverVersion;
}

String OctoPrinter::apiVersion() {
    return _server._apiVersion;
}

double OctoPrinter::progress() {
    return _job._progress;
}

bool OctoPrinter::closed() {
    return _is._closed;
}

bool OctoPrinter::startJob() {
    String response = _poster("/api/job", "{\"command\": \"start\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::cancelJob() {
    String response = _poster("/api/job", "{\"command\": \"cancel\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::restartJob() {
    String response = _poster("/api/job", "{\"command\": \"restart\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::pauseJob() {
    String response = _poster("/api/job", "{\"command\": \"pause\",\"action\": \"pause\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::resumeJob() {
    String response = _poster("/api/job", "{\"command\": \"pause\",\"action\": \"resume\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::toggleJobPauseState() {
    String response = _poster("/api/job", "{\"command\": \"pause\",\"action\": \"toggle\"}");
    if (response.toInt() == 204) return true;
    else return false;
}

bool OctoPrinter::gCode(String command) {
    String response = _poster("/api/printer/command", "{\"command\":\"" + command + "\"");
    if (response.toInt() == 204) return true;
    else return false;
}