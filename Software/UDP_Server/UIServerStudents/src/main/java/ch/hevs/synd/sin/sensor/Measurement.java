package ch.hevs.synd.sin.sensor;

import java.util.Date;

public class Measurement {
    public static final int TIMESTAMP_ROUND_FACTOR  = 25;
    public static final double VOLTAGE_VALUE_FACTOR = 230;
    public static final double CURRENT_VALUE_FACTOR = 15;

    private final MeasurementType   _type;
    private final double            _factor;
    private int                     _phase;
    private final int               _phaseShift;
    private double                  _value;
    private long                    _unixTimeStamp;


    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Constructors                                                                                                 */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    public Measurement(MeasurementType type, int phaseShift) {
        _type           = type;

        if (_type.equals(MeasurementType.Voltage)) {
            _factor     = VOLTAGE_VALUE_FACTOR;
        } else {
            _factor     = CURRENT_VALUE_FACTOR;
        }
        _phaseShift     = phaseShift;
        _value          = _getValue(0);
        _unixTimeStamp  = _getTimestamp();
    }


    /* ************************************************************************************************************ */
    /* Getter                                                                                                       */
    /* ************************************************************************************************************ */
    public MeasurementType getType() {
        return _type;
    }

    public long getUnixTimeStamp() {
        return _unixTimeStamp;
    }

    public double getValue() {
        return _value;
    }

    public int getPhase() {
        return _phase;
    }

    /* ************************************************************************************************************ */
    /* Public Section                                                                                               */
    /* ************************************************************************************************************ */
    public enum MeasurementType {
        Voltage,
        Current
    }

    public void setPhase(int phase) {
        _value          = _getValue(phase);
        _unixTimeStamp  = _getTimestamp();
    }

    /* ************************************************************************************************************ */
    /* Private methods                                                                                              */
    /* ************************************************************************************************************ */
    private long _getTimestamp() {
        return (new Date()).getTime() * TIMESTAMP_ROUND_FACTOR / TIMESTAMP_ROUND_FACTOR;
    }

    private double _getValue(int phase) {
        _phase = phase % 360;
        return Math.sin(Math.toRadians(phase + _phaseShift)) * _factor;
    }

}
