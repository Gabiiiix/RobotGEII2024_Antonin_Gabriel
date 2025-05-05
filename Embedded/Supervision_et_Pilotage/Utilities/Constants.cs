using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Constants
{
    public enum ObjectType
    {
        Unknown = 0,
        Robot = 1,
        Obstacle = 2,
        Beacon = 3,
        Target = 4,
        Wall = 5,
        Human = 6,
        Zone = 7,
        ObstacleLidar = 8
    }

    public enum AsservissementMode
    {
        DisabledM5 = 0,
        EnabledM5 = 1,

        DisabledM6 = 2,
        EnabledM6 = 3,

        DisabledM7 = 4,
        EnabledM7 = 5,

        DisabledM8 = 6,
        EnabledM8 = 7,

        Off2Wheels = 10,
        Polar2Wheels = 11,
        Independant2Wheels = 12,


        Off4Wheels = 13,
        Polar4Wheels = 14,
        Independant4Wheels = 15
    }
}


