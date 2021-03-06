// DeviceContext.cpp
//

#include "DeviceContext.h"

#define RADIUS 					(6371000) // 6371e3

#define TO_RADIAN(x)			((x)*(PI/180))
#define GET_DISTANCE(angle)		(2.0 * RADIUS * sin(TO_RADIAN((angle) / 2)))




////////////////////////////////////////////////////////////////////////////////////////////////
//

DeviceContext __DeviceContext;


const VarioTone DeviceContext::defaultTone[MAX_VARIO_TONE] =
#if 0
{
	{ -10.00,    200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,   490, 600,   30 },
	{    0.50,   740, 460,   50 },
	{    1.00,   860, 350,   60 },
	{    2.00,  1100, 320,   60 },
	{    3.00,  1200, 300,   60 },
	{    5.00,  1400, 250,   60 },
	{   10.00,  1570, 200,   60 },
};
#elif 0
{
	{ -10.00,    200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,   558, 600,   30 },
	{    1.00,   574, 525,   50 },
	{    2.00,   596, 450,   60 },
	{    4.00,   667, 300,   60 },
	{    6.00,   809, 267,   60 },
	{    8.00,  1082, 233,   60 },
	{   10.00,  1616, 200,   60 },
};
#else
// y = exp((x + 320) / 180) x 0.8 + 800
//
{
	{  -10.00,   200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,  1000, 600,   30 },
	{    1.00,  1100, 400,   50 },
	{    2.00,  1200, 300,   60 },
	{    4.00,  1400, 275,   60 },
	{    6.00,  1600, 250,   60 },
	{    8.00,  1800, 225,   60 },
	{   10.00,  2000, 200,   60 },
};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

DeviceContext::DeviceContext()
{
	reset();
}
	
void DeviceContext::reset()
{
	memset(this, 0, sizeof(DeviceContext));
	
	//
	gliderInfo.type = GTYPE_PARAGLIDER;
	
	memset(gliderInfo.manufacture, 0, sizeof(gliderInfo.manufacture));
	memset(gliderInfo.model, 0, sizeof(gliderInfo.model));
	
	//
	logger.enable = true;
	
	logger.takeoffSpeed = FLIGHT_START_MIN_SPEED;		// km/s
	logger.landingTimeout = FLIGHT_LANDING_THRESHOLD;	// ms
	logger.loggingInterval = FLIGHT_LOGGING_INTERVAL;	// ms
	
	memset(logger.pilot, 0, sizeof(logger.pilot));
	logger.timezone = VARIOMETER_TIME_ZONE; 			// GMT+9	
	
	//
	varioSetting.sinkThreshold = VARIOMETER_SINKING_THRESHOLD; // -3.0
	varioSetting.climbThreshold = VARIOMETER_CLIMBING_THRESHOLD; // 0.2
	varioSetting.sensitivity = VARIOMETER_SENSITIVITY; // 0.12
	
	varioSetting.sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;

	varioSetting.dampingFactor	= VARIOMETER_DEFAULT_DAMPING_FACTOR; // 0.1

	// vario_tone_table 
	memcpy(&toneTable[0], &defaultTone[0], sizeof(defaultTone));
	
	//
	volume.vario = VARIOMETER_BEEP_VOLUME;
	volume.effect = VARIOMETER_EFFECT_VOLUME;
	volume.autoTurnOn = 1;
	
	//
	threshold.lowBattery = LOW_BATTERY_THRESHOLD;
	threshold.shutdownHoldtime = SHUTDOWN_HOLD_TIME;
	threshold.autoShutdownVario = AUTO_SHUTDOWN_THRESHOLD;
	
	//
	kalman.varZMeas = KF_ZMEAS_VARIANCE;
	kalman.varZAccel = KF_ZACCEL_VARIANCE;
	kalman.varAccelBias = KF_ACCELBIAS_VARIANCE;

	//
	deviceDefault.enableBT = 1;
	deviceDefault.enableSound = 0;

	strcpy(deviceDefault.btName, "MiniVario");

	//
	deviceState.statusBT = deviceDefault.enableBT ? 1 : 0;

	//
	flightState.bearingTakeoff = -1;
}

bool DeviceContext::load(Preferences & pref)
{
	//
	pref.getBytes("vset", &varioSetting, sizeof(varioSetting));
	
	//
	pref.getBytes("ginfo", &gliderInfo, sizeof(gliderInfo));
	pref.getBytes("logger", &logger, sizeof(logger));
	
	//
	pref.getBytes("tones", &toneTable, sizeof(toneTable));

	//	
	pref.getBytes("volume", &volume, sizeof(volume));
	pref.getBytes("thshld", &threshold, sizeof(threshold));

	//	
	pref.getBytes("kman", &kalman, sizeof(kalman));
	
	// Device state
	pref.getBytes("device", &deviceDefault, sizeof(deviceDefault));

	return true;
}

bool DeviceContext::save(Preferences & pref)
{
	// 
	pref.putBytes("vset", &varioSetting, sizeof(varioSetting));
	
	//
	pref.putBytes("ginfo", &gliderInfo, sizeof(gliderInfo));
	pref.putBytes("logger", &logger, sizeof(logger));
	
	//
	pref.putBytes("tones", &toneTable, sizeof(toneTable));

	//	
	pref.putBytes("volume", &volume, sizeof(volume));
	pref.putBytes("thshld", &threshold, sizeof(threshold));

	//	
	pref.putBytes("kman", &kalman, sizeof(kalman));
	
	// Device state
	pref.putBytes("device", &deviceDefault, sizeof(deviceDefault));
}

#define VARIO_UPDATE_FREQ		(50)	// vario update every 20ms --> 50Hz
#define VARIO_AVERAGE_INTERVAL	(2)		// average for 2 seconds

void DeviceContext::updateVarioHistory()
{
	varioState.speedVertSumTotal += varioState.speedVertLazy; // varioState.speedVertActive;
	varioState.speedVertSumCount += 1;

	// 
	if (varioState.speedVertSumCount == VARIO_UPDATE_FREQ * VARIO_AVERAGE_INTERVAL) 
	{
		varioState.speedVertHistory[varioState.speedVertNext] = varioState.speedVertSumTotal / varioState.speedVertSumCount;
		//Serial.printf("#%d: %.1f -> %.1f\n", varioState.speedVertNext, varioState.speedVertSumTotal, varioState.speedVertHistory[varioState.speedVertNext]);

		varioState.speedVertSumTotal = 0.0;
		varioState.speedVertSumCount = 0;
		varioState.speedVertNext = (varioState.speedVertNext + 1) % MAX_VARIO_HISTORY;
	}
}

void DeviceContext::updateTrackHistory(float lat, float lon, float vario)
{
	// add new track point
	int nextPoint = (flightState.frontPoint + 1) % MAX_TRACK_HISTORY;

	if (nextPoint == flightState.rearPoint) // is full ?
		flightState.rearPoint = (flightState.rearPoint + 1) % MAX_TRACK_HISTORY;

	flightState.trackPoints[flightState.frontPoint].lat = lat;
	flightState.trackPoints[flightState.frontPoint].lon = lon;
	flightState.trackPoints[flightState.frontPoint].vario = vario;

	flightState.frontPoint = nextPoint;

	// calculate relative distance : distance from lastest point to each point
	int lastest = (flightState.frontPoint + MAX_TRACK_HISTORY - 1) % MAX_TRACK_HISTORY;

	for (int i = flightState.rearPoint; i != lastest; )
	{
		flightState.trackDistance[i].dx = GET_DISTANCE(flightState.trackPoints[lastest].lon - flightState.trackPoints[i].lon);
		flightState.trackDistance[i].dy = GET_DISTANCE(flightState.trackPoints[lastest].lat - flightState.trackPoints[i].lat);

		i = (i + 1) % MAX_TRACK_HISTORY;
	}

	flightState.trackDistance[lastest].dx = 0;
	flightState.trackDistance[lastest].dy = 0;	
}
