package ch.hevs.synd.sin.sensor;

import java.util.Timer;
import java.util.TimerTask;

public class Sensor {
    private final Measurement _measurement;
    private Timer _tMeasurement;

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Constructors                                                                                                 */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    public Sensor(Measurement.MeasurementType type, int phaseShift) {
        // Set up measurement of the sensor
        _measurement = new Measurement(type, phaseShift);

        // Set up a timer task
        _tMeasurement = new Timer();
        SensorTask sensorTask = new SensorTask(_measurement);
        _tMeasurement.scheduleAtFixedRate(sensorTask, 0, Measurement.TIMESTAMP_ROUND_FACTOR);
    }


    /* ************************************************************************************************************ */
    /* Getter                                                                                                       */
    /* ************************************************************************************************************ */
    public Measurement getMeasurement() {
        return _measurement;
    }


    /* ************************************************************************************************************ */
    /* Public Section                                                                                               */
    /* ************************************************************************************************************ */
    public void shutdown() {
        if (_tMeasurement != null) {
            _tMeasurement.cancel();
            _tMeasurement.purge();
        }
    }

    /* ************************************************************************************************************ */
    /* Private Section                                                                                              */
    /* ************************************************************************************************************ */
    private class SensorTask extends TimerTask {
        private Measurement _measurement;

        SensorTask(Measurement measurement) {
            _measurement = measurement;
        }

        public void run() {
            _measurement.setPhase(_measurement.getPhase() + 3);
        }
    }
}
