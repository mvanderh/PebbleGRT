// type location: {
//    lat: string,
//    lon: string,
// }

var DEFAULT_BUS_DISTANCE_LIMIT = 20; // km
var MAX_NUM_CLOSE_BUSES = 7;

var TYPE_KEY = '0';
var TYPE_VALUE_CLOSE_BUSES = 0;

function getGeoLocation(onSuccess) {
  var locationOptions = {
    enableHighAccuracy: true,
    maximumAge: 10000,
    timeout: 10000
  };
  navigator.geolocation.getCurrentPosition(function(pos) {
    var coords = pos.coords;
    onSuccess({
      lat: coords.latitude,
      lon: coords.longitude
    });
  }, locationError, locationOptions);
}

function locationError(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

function encodeBus(bus) {
  return bus.distance + "km;" + bus.description;
}

function reportClosestBuses() {
  function closeBusesCallback(buses) {
    var busStrings = buses.map(encodeBus);

    var msg = {
      TYPE_KEY: TYPE_VALUE_CLOSE_BUSES
    };
    extendWithArray(msg, busStrings, 1);

    Pebble.sendAppMessage(msg,
      function () {
        console.log("Message sent: \n" + JSON.stringify(msg));
      },
      function () {
        console.log("ERROR: could not send message: \n" + JSON.stringify(msg));
      });
  }

  getGeoLocation(function(myLoc) {
    GRT.getClosestBuses(myLoc, MAX_NUM_CLOSE_BUSES, closeBusesCallback);
  });
}

Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");
    reportClosestBuses();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    reportClosestBuses();
  }
);