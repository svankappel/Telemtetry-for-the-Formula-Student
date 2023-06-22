package ch.hevs.synd.sin.network.client.stream;

import ch.hevs.synd.sin.utils.Utility;

import java.io.IOException;
import java.io.OutputStream;

import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.sql.SQLOutput;
import java.util.Scanner;

public class UIClientTCP {

    public static void getValues(int z, int n, String server, int port)
    {
        String gu = "getu";     //strings of the messages
        String gi = "geti";
        try {
            Socket socket = new Socket(server, port);       //create the socket

            for(int i = 0; i<n;i++)         //loop to get many values
            {
                Utility.writeLine(socket.getOutputStream(), gu.getBytes());                             //send the getu message
                System.out.print(new String(Utility.readLine(socket.getInputStream()))+"       ");      //receive the answer and print it
                Utility.writeLine(socket.getOutputStream(), gi.getBytes());                             //send the geti message
                System.out.println(new String(Utility.readLine(socket.getInputStream())));              //receive the answer and print it

                Utility.waitSomeTime(z);    //wait
            }

            Utility.writeLine(socket.getOutputStream(), "stop".getBytes());     //send the stop message to end the connection

        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("Server can not be reached");
            System.exit(2);
            return;
        }
    }

    public static void main(String[] args){

        System.out.print("Enter number of values : ");      //get all the values from the user
        Scanner input = new Scanner(System.in);
        int n =new Integer(input.nextLine());
        System.out.print("Enter time (ms) : ");
        int z =new Integer(input.nextLine());
        System.out.println();
        System.out.println("Values :");

        getValues(z,n,"sdi.hevs.ch",1502);        
    }
}
