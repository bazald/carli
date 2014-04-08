package ch.idsia.ai.agents.ai;

import ch.idsia.ai.agents.Agent;
import ch.idsia.mario.environments.Environment;

/**
 * Created by IntelliJ IDEA.
 * User: julian
 * Date: Apr 28, 2009
 * Time: 2:09:42 PM
 */
public class JNIAgent implements Agent {

    static {
        System.loadLibrary("infinite_mario");
    }

    private native boolean[] c_getAction(Environment observation);
    private native void c_reset();



    private String name = "JNIAgent";
    final int numberOfInputs = 10;

    public JNIAgent () {
    }

    public void reset() {
      c_reset();
    }

    public boolean[] getAction(Environment observation) {
        byte[][] scene = observation.getLevelSceneObservation(/*1*/);
        // double[] inputs = new double[]{probe(-1, -1, scene), probe(0, -1, scene), probe(1, -1, scene),
                              // probe(-1, 0, scene), probe(0, 0, scene), probe(1, 0, scene),
                                // probe(-1, 1, scene), probe(0, 1, scene), probe(1, 1, scene),
                                // 1};
        return c_getAction(observation);
    }

    public AGENT_TYPE getType() {
        return AGENT_TYPE.AI;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    private double probe (int x, int y, byte[][] scene) {
        int realX = x + 11;
        int realY = y + 11;
        return (scene[realX][realY] != 0) ? 1 : 0;
    }
}
