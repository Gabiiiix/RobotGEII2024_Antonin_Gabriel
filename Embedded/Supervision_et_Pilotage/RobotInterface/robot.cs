using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RobotInterface
{
    public class Robot
    {
        public string receivedText = "";
        public float distanceTelemetreDroit;
        public float distanceTelemetreCentre;
        public float distanceTelemetreGauche;
        public Queue<byte> byteListReceived = new Queue<byte>();
        public float positionXOdo;
        public float positionYOdo;
        public float angleRadian;
        public float vitesseLineaire;
        public float vitesseAngulaire;
        public float vitesseGauche;
        public float vitesseDroit;
        public long time;
        public Robot()
        {

        }
    }
}