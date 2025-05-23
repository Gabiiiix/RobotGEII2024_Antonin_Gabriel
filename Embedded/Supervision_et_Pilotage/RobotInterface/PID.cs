﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RobotInterface
{
    public class PID
    {
        public double Kp;
        public double Ki;
        public double Kd;
        public double ErreurP;
        public double ErreurI;
        public double ErreurD;
        public double ErreurPMax;
        public double ErreurIMax;
        public double ErreurDMax;

        public PID() { 
        }
    }

    public enum PIDType
    {
        LINEAIRE = 0,
        ANGULAIRE = 1
    }
}
