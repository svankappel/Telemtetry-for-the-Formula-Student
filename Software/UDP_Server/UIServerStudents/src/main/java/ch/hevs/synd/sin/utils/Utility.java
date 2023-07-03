package ch.hevs.synd.sin.utils;

import java.io.*;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Random;

/**
 * This class contains some useful Java methods to manipulate data.
 *
 * @author Michael Clausen
 * @author Patrice Rudaz
 */
@SuppressWarnings("unused")
public class Utility {
    /** Default size for the TCP input stream */
    public static final int	TCP_BUFFER_SIZE 	= 4096;

    /** Object to get some random values... */
    public static Random rnd = new Random(1);

    /**
     * Calculates and returns the CRC using the data passed in parameters.
     *
     * @param data a byte array containing the data to send
     * @param offset the offset
     * @param len the data length
     * @return byte[] the CRC
     */
    public static byte[] calculateCRC(byte[] data , int offset , int len)
    {
        int crc = 0x0000FFFF;
        for (int i = 0 ; i < len ; i++) {
            crc = crc ^ Utility.unsignedByteToSignedInt(data[i + offset]);
            for (int j = 0 ; j < 8 ; j++) {
                int tmp = crc;
                int carryFlag = tmp & 0x0001;
                crc = crc >> 1;
                if (carryFlag == 1) {
                    crc = crc ^ 0xA001;
                }
            }
        }

        byte[] result = new byte[2];
        result[0] = (byte)(crc & 0xFF);
        result[1] = (byte)((crc & 0xFF00) >> 8);

        return result;
    }

    /**
     * Checks the CRC and returns true if it is correct, otherwise false.
     *
     * @param data a byte array containing the data to send
     * @param offset the offset
     * @param len the data length
     * @param crc a byte array containing the CRC to check
     * @return boolean true if the CRC is correct, otherwise false
     */
    public static boolean checkCRC(byte[] data , int offset , int len , byte[] crc)
    {
        byte[] calcCrc = Utility.calculateCRC(data , offset , len);
        return (calcCrc[0] == crc[0] && calcCrc[1] == crc[1]);
    }


    /**
     * Returns a <code>float</code> value from array of bytes. This byte's array can only be 2 or 4 bytes long.
     * @param bytes     The array of bytes to convert.
     * @param offset    The position where the method has to start to get the bytes from.
     * @param size      The amount of bytes to convert.
     * @return A <code>Float</code> value or <code>null</code> if the process failed.
     */
    public static Float bytesToFloat(byte[] bytes, int offset, int size) {

        if (size == 2 || size == 4) {
            byte[] tmp = new byte[4];
            System.arraycopy(bytes, offset, tmp, 0, size);
            try {
                return ByteBuffer.wrap(tmp).order(ByteOrder.BIG_ENDIAN).getFloat();
            } catch (Exception e) {
                DEBUG("utils", "Utility", "bytesToFloat()", "ByteBufferException : " + e.getMessage());
            }
        } else {
            DEBUG("utils", "Utility", "bytesToFloat()", "ERROR: size MUST be 2 or 4 !!!");
        }
        return null;
    }

    /**
     * Converts an unsigned byte to a signed integer.
     * @param from an unsigned byte to convert to a signed integer
     * @return int a signed integer
     */
    public static int unsignedByteToSignedInt(byte from)
    {
        return 0x000000FF & (int)from;
    }

    /**
     * Utility method to convert a byte array in a string made up of hex (0,.. 9, a,..f)
     * @param b The array of bytes to be convert in HEX characters.
     * @return A string representing the given array of byte in HEX values.
     */
    public static String getHexString(byte[] b) {
        return getHexString(b, 0, b.length);
    }

    /**
     * Utility method to convert a byte array in a string made up of hex (0,.. 9, a,..f)
     * @param b			The byte array to convert in HEX string
     * @param offset	The index where we start to convert from.
     * @param length	The amount of bytes to convert in HEX string
     * @return A String representing the HEX values of the selected bytes of the array.
     */
    public static String getHexString(byte[] b, int offset, int length) {
        StringBuilder result = new StringBuilder();
        for (int i = offset; i < offset+length; i++) {
            result.append(Integer.toString((b[i] & 0xff) + 0x100, 16).substring(1));
        }
        return result.toString();
    }

    /**
     * Utility method to convert a byte array in a string made up of hex (0,.. 9, a,..f) and
     * format the string with `0xCC` where CC is the string in HEX
     * @param b			The byte array to convert in HEX string
     * @param offset	The index where we start to convert from.
     * @param length	The amount of bytes to convert in HEX string
     * @return A String representing the HEX values of the selected bytes of the array.
     */
    public static String getHexStringForDebug(byte[] b, int offset, int length) {
        StringBuilder result = new StringBuilder();
        for (int i = offset; i < offset+length; i++) {
            result.append("0x");
            result.append(Integer.toString((b[i] & 0xff) + 0x100, 16).substring(1));
            result.append(" ");
        }
        return result.toString().trim();
    }


    /**
     * To be used to make a thread waiting for a certain amount of time
     * @param thread	The thread to be stopped
     * @param millis	The amount of time in [ms] to stop the thread.
     */
    public static void threadWait(Thread thread, long millis) {
        try {
            thread.wait(millis);
        } catch(InterruptedException ie) {
            DEBUG("utils", "Utility", "threadWait()", "InterruptedException : " + ie.getMessage());
        } catch (NullPointerException npe) {
            DEBUG("utils", "Utility", "threadWait()", "The given Thread is NULL !");
        }
    }

    /**
     * To wait some times ...
     *
     * @param ms The milliseconds to wait...
     */
    public static void waitSomeTime(int ms)
    {
        try {
            Thread.sleep(ms);
        } catch(InterruptedException e) {
            DEBUG("utils", "Utility", "waitSomeTime()", "InterruptedException : " + e.getMessage());
        }
    }


    /**
     * Returns the md5 of any input...
     * @param msg The input string to process
     * @return  The md5 of the input string.
     */
    public static String md5sum(String msg)
    {
        try {
            MessageDigest md = MessageDigest.getInstance("md5");
            return String.format("%032x", new BigInteger(1, md.digest(msg.getBytes(StandardCharsets.UTF_8))));
        } catch (NoSuchAlgorithmException e) {
            DEBUG("utils", "Utility", "md5sum()", "NoSuchAlgorithmException : " + e.getMessage());
        }
        return null;
    }

    /**
     * Retrieves a random value rounded to 2 decimal...
     *
     * @param factor A coefficient which the random value is multiplied with.
     * @return A random value rounded to 2 decimal converted to a String.
     */
    public static String getStringRndVal(int factor) {
        float val = (float) rnd.nextDouble()*factor*10;
        return String.format("%.2f", val).replace(",", ".");
    }


    /**
     * Reads the incoming data from an input stream as long as there is
     * something to read and saved the data in an array of bytes.
     *
     * The method is blocking !
     * This method blocks until input data is available, end of file is detected,
     * or an exception is thrown.
     *
     * If the length of of read data is zero, then no bytes are read and an empty
     * array of bytes is returned; otherwise, there is an attempt to read at least
     * one byte. If no byte is available because the stream is at the end of the
     * file, the value -1 is returned; otherwise, at least one byte is read and
     * returned as an array of bytes.
     *
     * @param  in	The input Stream where to read the data coming from.
     * @return The read data as an <code>array of bytes</code>. Or null if the
     * 		   has been closed by the peer while waiting for incoming data.
     * @throws IOException  If the first byte cannot be read for any reason other
     *                      than the end of the file, if the input stream has been
     *                      closed, or if some other I/O error occurs.
     */
    public static byte[] readBytes(InputStream in) throws IOException
    {
        byte[] buffer = new byte[TCP_BUFFER_SIZE];

        // Read the incoming data
        int b = in.read(buffer);

        // Creates an array of bytes with the right size
        if (b == -1)    return null;		// the connection has been canceled by the peer
        if (b ==  0)    return new byte[0];	// empty data
        else {                      		// The received data
            byte[] rBytes = new byte[b];
            System.arraycopy(buffer, 0, rBytes, 0, b);
            return rBytes;
        }
    }

    /**
     * Reads from the given input stream an amount of bytes and retrieves these data as
     * an array of bytes. This method is
     *
     * The method is blocking !
     *
     * @param in	The input Stream where to read the data coming from.
     * @param len   The number of bytes to be read from the input stream.
     *
     * @return	The read data as an <code>array of bytes</code>.
     *
     * @throws IOException  If the first byte cannot be read for any reason other
     *                      than the end of the file, if the input stream has been
     *                      closed, or if some other I/O error occurs.
     */
    public static byte[] readNBytes(InputStream in, int len) throws IOException
    {
        byte[] buffer = new byte[len];

        // Checks that there is enough available data to read from the input stream
        int counter = 0;
        while (in.available() < len) {
            Utility.waitSomeTime(2);
            if (++counter > 10)
                return null;
        }

        // Read the incoming data
        int b = in.read(buffer, 0, len);

        // Creates an array of bytes with the right size
        if (b == -1)        return null;		// the connection has been canceled by the peer
        else if (b == 0)    return new byte[0];	// empty data
        else                return buffer;		// The received data
    }

    /**
     * Reads a line of text. A line is considered to be terminated by any one of a
     * line feed ('\n'), a carriage return ('\r'), or a carriage return followed
     * immediately by a line feed.
     *
     * @param in The Input Stream to read from.
     * @return An array of bytes containing the contents of the line, not including any
     * 	       line-termination characters, or null if the end of the stream has
     *         been reached.
     * @throws IOException	If an I/O error occurs
     */
    public static byte[] readLine(InputStream in) throws IOException
    {
        BufferedReader reader = new BufferedReader(new InputStreamReader(in));

        // Read a complete line with Cariage Return and/or Line Feed
        String line = reader.readLine();

        // Handle the value to return
        if (line != null)
            return line.replace("\n", "").replace("\r", "").getBytes();
        else
            return null;
    }

    /**
     * Send the data contained in the given array of bytes through the output
     * stream. It raises an IOException if something goes wrong.
     * The method returns the amount of bytes sent.
     *
     * @param out		The Output Stream to send the data to.
     * @param toSend	The data to send
     * @throws IOException	If an I/O error occurs
     */
    public static void writeLine(OutputStream out, byte[] toSend) throws IOException {

        if (toSend[toSend.length - 1] == '\n')
            out.write(toSend);
        else {
            byte[] temp = new byte[toSend.length + 2];
            System.arraycopy(toSend, 0, temp, 0, toSend.length);
            temp[temp.length - 2] = (byte) '\r';
            temp[temp.length - 1] = (byte) '\n';
            out.write(temp);
        }

        out.flush();
    }


    /**
     * Method to use to access a file in your resources folder...
     * @param pathName  folder's name (if exists) from `resources`
     * @param fileName  Name of the file to access
     *
     * @return A BufferedReader related to the file you want to work with, or null if the file could not be reached.
     */
    public static BufferedReader fileParser(String pathName, String fileName) {
        // set relative path
        InputStream is;
        if (pathName == null) {
            is = ClassLoader.getSystemClassLoader().getResourceAsStream(fileName);
        } else {
            is = ClassLoader.getSystemClassLoader().getResourceAsStream(pathName + "/" + fileName);
        }

        if (is != null) {
            return new BufferedReader(new InputStreamReader(is));
        }
        return null;
    }


    // DEBUG System.out
    public static void DEBUG(String className, String method, String msg) {
        DEBUG(null, className, method, msg);
    }

    public static void DEBUG(Object object, String method, String msg) {
        String className    = object.getClass().getSimpleName();
        String[] packages   = object.getClass().getPackage().getName().split("\\.");
        if (packages.length > 1)
            DEBUG(packages[packages.length-1], className, method, msg);
        else if (packages.length == 1)
            DEBUG(packages[0], className, method, msg);
        else
            DEBUG(null, className, method, msg);
    }

    public static void DEBUG(String packageName, String className, String method, String msg) {
        final int nameLength = 25;
        int millis = Calendar.getInstance().get(Calendar.MILLISECOND);
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");

        className = className.trim().replace("[", "").replace("]", "");
        if (packageName != null)
            className = packageName + ":" + className;

        String cn = " [" + className + "] ";
        while (cn.length() < nameLength) {
            cn += " ";
        }
        StringBuilder logMsg = new StringBuilder();

        // set the time
        logMsg.append(sdf.format(Calendar.getInstance().getTime())).append(".").append(String.format("%03d", millis));

        // Append Packages and ClassName
        logMsg.append(cn);

        // Append Method
        method  = method.trim().replace("(", "").replace(")", "");
        method += "() > ";
        while (method.length() < nameLength) {
            method += " ";
        }
        logMsg.append(method);

        // Append message if exists
        if (msg != null) {
            logMsg.append(msg);
        }

        if (logMsg.toString().endsWith("\n")) {
            System.out.print(logMsg);
        } else {
            System.out.println(logMsg);
        }
    }
}
