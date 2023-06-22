package ch.hevs.synd.sin.network.Datagram;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;


public class UIClientUDP {


    // Function that request the value and print the answer on the console
    public static void getValues(String server, int port, String message)
    {
        try {

            DatagramSocket socket = new DatagramSocket();                   //creates a new datagram socket
            socket.setSoTimeout(16000);                                     //set timeout value

            //create the packet with the message, length, address and port
            DatagramPacket packet = new DatagramPacket(message.getBytes(),message.getBytes().length, InetAddress.getByName(server),port);

            socket.send(packet);                            //send the packet

            byte[] rxBuf = new byte[256];                   // create a buffer to put the answer in

            //create a packet for the rx message
            DatagramPacket rxDatagramPacket = new DatagramPacket(rxBuf, rxBuf.length,InetAddress.getByName(server),port);

            socket.receive(rxDatagramPacket);       //receive the packet
            System.out.print(new String(rxBuf,0, rxDatagramPacket.getLength()));    //print the content


        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("Server can not be reached");
            System.exit(2);
            return;
        }

    }

    public static void main(String[] args){

        System.out.println("Values from getu :");       //print message

        getValues("localhost",1502, "getu");    //get values
    }


}
