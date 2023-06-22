package ch.hevs.synd.sin.network.server;

import ch.hevs.synd.sin.sensor.Measurement;
import ch.hevs.synd.sin.sensor.Sensor;
import ch.hevs.synd.sin.utils.Utility;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class Transaction {

    private final InputStream _in;
    private final OutputStream _out;

    private final Sensor _uSensor;
    private final Sensor _iSensor;


    /* **************************************************************************************************************** */
    /*                                                                                                                  */
    /* Constructors                                                                                                     */
    /*                                                                                                                  */
    /* **************************************************************************************************************** */
    /**
     * Simple constructor
     * @param in		The input stream to read from
     * @param out		The output stream to write to
     * @param uSensor	Pointer to the Voltage sensor
     * @param iSensor	Pointer to the Ampere sensor
     */
    public Transaction(InputStream in, OutputStream out, Sensor uSensor, Sensor iSensor) {
        _in = in;
        _out = out;
        _uSensor = uSensor;
        _iSensor = iSensor;
    }


    /* **************************************************************************************************************** */
    /*                                                                                                                  */
    /* Public methods                                                                                                   */
    /*                                                                                                                  */
    /* **************************************************************************************************************** */
    public boolean processTransaction() throws IOException {

        boolean process = transaction(new String(Utility.readLine(_in)));       //call the private method

        return process;
    }


    /* **************************************************************************************************************** */
    /*                                                                                                                  */
    /* Private methods                                                                                                  */
    /*                                                                                                                  */
    /* **************************************************************************************************************** */
    // Private method which will interpret the given command and react in to it...
    private boolean transaction(String dataIn) throws IOException {
        boolean b;
        switch (dataIn)         //switch on the inputstream converted into a string
        {
            case "stop" : b = false;        //return false to finish the connection
            break;
            case "getu" : Utility.writeLine(_out, String.format("%.2f [V]",_uSensor.getMeasurement().getValue()).getBytes());   //send sensor value
            b = true;
            break;
            case "geti" : Utility.writeLine(_out, String.format("%.2f [A]",_iSensor.getMeasurement().getValue()).getBytes());   //send sensor value
            b = true;
            break;
            default : Utility.writeLine(_out, "Incorrect Input".getBytes());    //send a message if the input was incorrect
            b = true;
        }
        return b;   //return false if connection is finished

    }


    /* **************************************************************************************************************** */
    /*                                                                                                                  */
    /* MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN -- MAIN --  */
    /*                                                                                                                  */
    /* **************************************************************************************************************** */
   /* public static void main(String[] args) throws IOException {

        Sensor uSensor = new Sensor(Measurement.MeasurementType.Voltage,0);
        Sensor iSensor = new Sensor(Measurement.MeasurementType.Current,0);
        Transaction transaction = new Transaction(System.in,System.out,uSensor,iSensor);
        while(transaction.processTransaction()){}

        System.exit(2);
    }*/
}
