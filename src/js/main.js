/**
 * Created by mitch on 15-02-07.
 */

var PGTypeReportCloseBuses = 0;
var PGTypeBusDetail = 1;
var PGTypeReportCloseStops = 2;

function getGeoLocation(onSuccess) {
    var locationOptions = {
        enableHighAccuracy: true,
        maximumAge: 10000,
        timeout: 10000
    };
    navigator.geolocation.getCurrentPosition(function (pos) {
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
    return [
        bus.distance + "km",
        bus.description,
        bus.tripId,
        bus.vehicleId
    ].join(";");
}

function sendJsonToPebble(json) {
    var string = JSON.stringify(json);
    console.log("Sending message to Pebble: " + string);

    Pebble.sendAppMessage(msg,
        function () {
            console.log("Message sent.");
        },
        function () {
            console.log("ERROR: could not send message: \n" + string);
        });
}

function reportClosestBuses() {
    function closeBusesCallback(buses) {

    }

    getGeoLocation(function (loc) {
        GRT.findNearbyBuses(loc, closeBusesCallback);
    });
}

function busDetail(vehicleId, tripId) {
    console.log("Getting bus detail for vehicleId=" + vehicleId + " tripId=" + tripId);

    function busInfoCallback(info) {
        var msg = {
            "PGKeyMessageType": 1,
            "PGKeyBusDetailDelay": info.delay
        };
        extendWithArray(msg, info.stops, 2);

        sendJsonToPebble(msg);
    }

    getGeoLocation(function (loc) {
        GRT.getBusInfo(loc, vehicleId, tripId, busInfoCallback);
    });
}

Pebble.addEventListener('appmessage',
    function (e) {
        var data = e.payload;
        console.log("Received message from Pebble: " + JSON.stringify(data));
        var messageType = data["PGKeyMessageType"];
        switch (messageType) {
            case PGTypeReportCloseBuses:
                reportClosestBuses();
                break;
            case PGTypeBusDetail:
                var vehicleId = data["PGKeyVehicleId"];
                var tripId = data["PGKeyTripId"];
                busDetail(vehicleId, tripId);
                break;
            case PGTypeReportCloseStops:

                break;
        }
    }
);