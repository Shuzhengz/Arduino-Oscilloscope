import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Calculate the speed of light based on the apparatus
 * Calibrate your apparatus before use!
 */
public class Main {
    public static double c;               // calculated speed of light           (m/s)
    public static double t;               // the travel time of light            (s)
    public static double l;               // length of the fiber                 (m)
    public static final double n = 1.5;   // Index of refraction of the fiber

    public static void main(String args[]) throws IOException{

        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        System.out.println("Length of the fiber:");
        String fiberLength = reader.readLine();
        l = Double.parseDouble(fiberLength);

        System.out.println("Time Delay of the Light:");
        String timeDelay = reader.readLine();
        t = Double.parseDouble(timeDelay);

        /**
         * Snell's law 
         * n1 * sin(theta1) = n2 * sin(theta2)
         * 
         * Speed of light in medium
         * n = c/v
         * 
         * the index of refraction of a particular
         * medium equals the speed of light in a
         * vacuum divided by the speed of light in
         * the medium
         */
        c = (n * l) / t;
 
        System.out.println(c);

        // The speed of light is supposed to be 3.00E8 m/s
    }
}
