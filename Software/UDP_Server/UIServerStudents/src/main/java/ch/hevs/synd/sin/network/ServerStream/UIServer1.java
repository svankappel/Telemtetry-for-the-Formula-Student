package ch.hevs.synd.sin.network.ServerStream;

import ch.hevs.synd.sin.sensor.Measurement;
import ch.hevs.synd.sin.sensor.Sensor;
import ch.hevs.synd.sin.utils.Utility;
import com.sun.security.ntlm.Server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class UIServer1 extends ServerSocket {


    UIServer1(int port) throws IOException {
        super(port);
    }



    public static void main(String[] args){

        try
        {
            Sensor uSensor = new Sensor(Measurement.MeasurementType.Voltage,0);     //create the new sensors
            Sensor iSensor = new Sensor(Measurement.MeasurementType.Current,0);     

            ServerSocket ss =  new ServerSocket(1502);          //create the socket

            while(true)
            {

                ConnectionHandler1 ch1 = new ConnectionHandler1(ss.accept(),uSensor,iSensor);       //create a connection handler

            }


        }
        catch(IOException e)
        {
            e.printStackTrace();

            System.exit(2);
            return;
        }

    }

}
