package ch.hevs.synd.sin.network.ServerStream;

import ch.hevs.synd.sin.network.server.Transaction;
import ch.hevs.synd.sin.sensor.Sensor;

import java.io.IOException;
import java.net.Socket;

public class ConnectionHandler1 {
    public ConnectionHandler1(Socket accept, Sensor uSensor, Sensor iSensor) throws IOException {

        Transaction transaction = new Transaction(accept.getInputStream(), accept.getOutputStream(), uSensor,iSensor);      //do a transaction

        while(transaction.processTransaction()){}   //wait until transaction is finished
        accept.close();

    }
}
