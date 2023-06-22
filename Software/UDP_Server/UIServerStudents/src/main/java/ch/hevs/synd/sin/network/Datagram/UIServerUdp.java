package ch.hevs.synd.sin.network.Datagram;

import ch.hevs.synd.sin.sensor.Measurement;
import ch.hevs.synd.sin.sensor.Sensor;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class UIServerUdp {

    public static void main(String[] args) {

        try {

            DatagramSocket socket = new DatagramSocket(1502);   //create the socket

            while(true)
            {
                byte[] rxBuf = new byte[2048];   //create a buffer to put the rx message in
                DatagramPacket rxDatagramPacket = new DatagramPacket(rxBuf, rxBuf.length);  //create a packet for the rx message
                socket.receive(rxDatagramPacket);   //receive the message
                String rx = new String(rxBuf);      //convert the byte array in a string
                System.out.println(rx);             //print string message
            }
        } catch (IOException e)
        {
            e.printStackTrace();
            System.out.println("Server can not be reached");
            System.exit(2);
        }
    }
}
