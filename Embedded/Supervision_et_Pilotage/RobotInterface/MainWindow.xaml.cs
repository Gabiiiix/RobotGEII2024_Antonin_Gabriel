using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using ExtendedSerialPort_NS;
using System.Windows.Threading;
using System.IO.Ports;
using System.Reflection.Metadata;
using System.Security.RightsManagement;
using static SciChart.Drawing.Utility.PointUtil;
using SciChart.Charting.Visuals;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.ComponentModel;
using WpfOscilloscopeControl;
using Constants;
using Newtonsoft.Json.Linq;
using SciChart.Core.Extensions;
using SciChart.Data.Model;
using Microsoft.VisualBasic;
using MathNet;
using MathNet.Numerics.Distributions;
namespace RobotInterface

{

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public partial class MainWindow : Window
    {

        ExtendedSerialPort serialPort1;
        DispatcherTimer timerAffichage;
        DispatcherTimer timerMap;

        Robot robot = new Robot();

        PID PIDLineaire = new PID();
        PID PIDAngulaire = new PID();

        
        
        public MainWindow()
        {
            // Set this code once in App.xaml.cs or application startup
            SciChartSurface.SetRuntimeLicenseKey("VKOUDZGU6WndydcBQTqx4px2yWsaXqbn+hIKIxA5AE7Vii9ai5FosulEM8j2NYkBkJFZ6Ei2pFlUIV8aoE7bc3FfN3QRUwtvCaGqmrseTOeNsCz9p4t2CBk7TjcTPW7JTOYnIH/UjoRxi8b0BK6MDi8XJUS98gXSybDb/cn070Y5voaiKvusgmvvAOjcwuGcPQuyV7vJlzqh3LqLL3TqJnJMTdGmM00s8VFb7U+sxfbzT/h8SQuY13u/3i5sSz0VEI6YYJeiiX3oMajfHwA/SGyyDFTZmDAAfILtohF7ag+hnEpUDqhudgYjXqVwVtc0oUZNT8Ghtx0ek2bjkQukPtp8/44M1wiOdZORUOCAxeh3oTPZKjEGRjkpbN/UKprgi8/Xvf11BuXzTJLXklmSZLFRsgxcx3nvQVwae9oY5HABtwOk+q/bdsNBKyPmhjNLM1+y5qSlpIQlHzm/EdvN44AX5iR43d4dxfLx9QN7KHvaUbHpqNXVKLUsq0g1g6mEGntw5fXj");
            


            InitializeComponent();

            serialPort1 = new ExtendedSerialPort("COM11", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();

            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();
            
            timerMap = new DispatcherTimer();
            timerMap.Interval = new TimeSpan(0,0,0, 0, 10);
            timerMap.Tick += TimerMap_Tick;
            timerMap.Start(); 

            oscilloSpeed.isDisplayActivated = true;
            oscilloSpeed.AddOrUpdateLine(1, 100, "VGauche");
            oscilloSpeed.AddOrUpdateLine(2, 100, "VDroit");
            oscilloSpeed.ChangeLineColor(1, Colors.Red);
            oscilloSpeed.ChangeLineColor(2, Colors.Blue);

            asservDisplay.SetAsservissementMode(AsservissementMode.Polar2Wheels);

            PIDLineaire.Kp = 2.0f;
            PIDLineaire.Ki = 20.0f;
            PIDLineaire.Kd = 0;
            PIDLineaire.ErreurPMax = 100.0f;
            PIDLineaire.ErreurIMax = 40.0f;
            PIDLineaire.ErreurDMax = 100.0f;

            PIDAngulaire.Kp = 0.75f;
            PIDAngulaire.Ki = 10.0f;
            PIDAngulaire.Kd = 0;

            PIDAngulaire.ErreurPMax = 100.0f;
            PIDAngulaire.ErreurIMax = 20.0f;
            PIDAngulaire.ErreurDMax = 100.0f;

            robot.angleActuelGhost = 90;
            robot.angleCible = 90;
            robot.positionXGhost = 20;
            robot.positionYGhost = 50;
            

            robot.ghostSpeedDegree = 1;
            robot.ghostSpeedLinaire = 1;
            robot.ghostAccelerationLineaire = 0.5;





        }

        private void TimerAffichage_Tick(object sender, EventArgs e)
        {


            oscilloSpeed.AddPointToLine(1, robot.time, robot.vitesseGauche);
            oscilloSpeed.AddPointToLine(2, robot.time, robot.vitesseDroit);
            oscilloSpeed.AddPointToLine(3, robot.time, robot.vitesseLineaire);

            asservDisplay.UpdatePolarOdometrySpeed(robot.positionXOdo, robot.vitesseAngulaire);
            asservDisplay.UpdateIndependantOdometrySpeed(robot.vitesseDroit, robot.vitesseGauche);
            asservDisplay.UpdatePolarSpeedCorrectionLimits(PIDLineaire.ErreurPMax, PIDAngulaire.ErreurPMax, PIDLineaire.ErreurIMax, PIDAngulaire.ErreurIMax, PIDLineaire.ErreurDMax, PIDAngulaire.ErreurDMax);
            asservDisplay.UpdatePolarSpeedConsigneValues(PIDLineaire.ConsigneDebug, PIDAngulaire.ConsigneDebug);
            asservDisplay.UpdatePolarSpeedErrorValues(PIDLineaire.Erreur, PIDAngulaire.Erreur);
            asservDisplay.UpdatePolarSpeedCorrectionValues(PIDLineaire.CorrP, PIDAngulaire.CorrP, PIDLineaire.CorrI, PIDAngulaire.CorrI, PIDLineaire.CorrD, PIDAngulaire.CorrD);

            asservDisplay.UpdateDisplay();

            LabelX.Text = "X: " + robot.angleRadian.ToString() + " cm";
            LabelY.Text = "Y: " + robot.positionYOdo.ToString() + " cm";


            
            // asservDisplay.UpdatePolarSpeedCommandValues()

            //checkBoxLEDRouge.IsChecked = EtatLEDRouge;
            //checkBoxLEDVerte.IsChecked = EtatLEDVerte;
            //checkBoxLEDBlanche.IsChecked = EtatLEDBlanche;
            //checkBoxLEDBleue.IsChecked = EtatLEDBleue.
            //checkBoxLEDOrange.IsChecked = EtatLEDOrange;QW
        }

        private void TimerMap_Tick(object sender, EventArgs e)
        {

            worldMap.UpdatePosRobot(robot.positionXOdo * 10 + 20, robot.positionYOdo * 10 + 50);
            worldMap.UpdatePosGhost(robot.positionXGhost * 10 + 20, robot.positionYGhost * 10 + 50);
            worldMap.RotateGhost(robot.angleActuelGhost);
            
        }




        public void SerialPort1_DataReceived(object sender, DataReceivedArgs e)
        {
            //TextBoxReception.Text = Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            //robot.receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            for (int i = 0; i < e.Data.Length; i++)
                // robot.byteListReceived.Enqueue(e.Data[i]);
                DecodeMessage(e.Data[i]);

        }

        private void boutonTest_Click(object sender, RoutedEventArgs e)
        {
            #region Ancient test
            // byte[] byteList = new byte [20];

            // for (int i = 0; i < 20; i++)
            //{
            //    byteList[i] = (byte)(2 * i);
            //}
            //serialPort1.Write(byteList, 0, byteList.Length);
            #endregion

            int msgFunction = 0x0080;
            string s = "Bonjour";
            byte[] array = Encoding.ASCII.GetBytes(s);
            int msgLength = array.Length;
            //UartEncodeAndSendMessage(msgFunction, msgLength, array);

            msgFunction = 0x0030;
            for (int i = 0;i < msgLength; i++)
            {
                array[i] = (byte)((i + 1) * 3 - 2);
            }
            msgLength = 5;


            UartEncodeAndSendMessage(msgFunction, msgLength, array);


        }

        private void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            int msgLength = msgPayloadLength + 6;
            byte[] msg = new byte[msgLength];

            msg[0] = 0xFE;
            msg[1] = (byte)(msgFunction >> 8);
            msg[2] = (byte)(msgFunction & 0x00FF);
            msg[3] = (byte)(msgPayloadLength >> 8);
            msg[4] = (byte)(msgPayloadLength & 0x00FF);
            for (int i = 0; i < msgPayloadLength; i++)
            {
                msg[i + 5] = msgPayload[i];
            }
            msg[msgLength - 1] = CalculateChecksum(msgFunction, msgPayloadLength, msgPayload);
            
            serialPort1.Write(msg, 0, msgLength); 
        }

        private byte CalculateChecksum(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            byte checksum = 0xFE;
            checksum = (byte)(checksum ^ (byte)(msgFunction) >> 8);
            checksum = (byte)(checksum ^ (byte)(msgFunction) & 0x00FF);
            checksum = (byte)(checksum ^ (byte)(msgPayloadLength) >> 8);
            checksum = (byte)(checksum ^ (byte)(msgPayloadLength) & 0x00FF);

            for (int i = 0; i < msgPayloadLength; i++)
            {
                checksum = (byte)(checksum ^ msgPayload[i]);
            }

            return checksum;
        }

        public enum StateReception
        {
            Waiting,
            FunctionMSB,
            FunctionLSB,
            PayloadLengthMSB,
            PayloadLengthLSB,
            Payload,
            CheckSum
        }


        StateReception rcvState = StateReception.Waiting;
        int msgDecodedFunction = 0;
        int msgDecodedPayloadLength = 0;
        byte[] msgDecodedPayload = new byte[256];
        int msgDecodedPayloadIndex = 0;

        int robotState = 0;
        long instant = 0 ;
        long instant_1 = 0;

        int distanceIRExtremeDroite = 0;
        int distanceIRExtremeGauche = 0;
        int distanceIRGauche = 0;
        int distanceIRMilieu = 0;
        int distanceIRDroite = 0;

        bool EtatLEDVerte = false;
        bool EtatLEDRouge = false;
        bool EtatLEDOrange = false;
        bool EtatLEDBleue = false;
        bool EtatLEDBlanche = false;

        public enum StateRobot
        {
            STATE_ATTENTE = 0,
            STATE_ATTENTE_EN_COURS = 1,
            STATE_AVANCE = 2,
            STATE_AVANCE_EN_COURS = 3,
            STATE_TOURNE_GAUCHE = 4,
            STATE_TOURNE_GAUCHE_EN_COURS = 5,
            STATE_TOURNE_DROITE = 6,
            STATE_TOURNE_DROITE_EN_COURS = 7,
            STATE_TOURNE_SUR_PLACE_GAUCHE = 8,
            STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS = 9,
            STATE_TOURNE_SUR_PLACE_DROITE = 10,
            STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS = 11,
            STATE_ARRET = 12,
            STATE_ARRET_EN_COURS = 13,
            STATE_RECULE = 14,
            STATE_RECULE_EN_COURS = 15,
            STATE_TOURNE_LENTEMENT_DROITE = 16,
            STATE_TOURNE_LENTEMENT_GAUCHE = 17
        }


        private void DecodeMessage(byte c)
        {
            switch (rcvState)
            {
                case StateReception.Waiting:
                    if (c == 0xFE)
                    {
                        rcvState = StateReception.FunctionMSB;
                    }
                    break;

                case StateReception.FunctionMSB:
                    msgDecodedFunction = c << 8;
                    rcvState = StateReception.FunctionLSB;
                    break;

                case StateReception.FunctionLSB:
                    msgDecodedFunction = msgDecodedFunction | c;
                    rcvState = StateReception.PayloadLengthMSB;
                    break;

                case StateReception.PayloadLengthMSB:
                    msgDecodedPayloadLength = c << 8;
                    rcvState = StateReception.PayloadLengthLSB;
                    break;

                case StateReception.PayloadLengthLSB:
                    msgDecodedPayloadLength = msgDecodedPayloadLength | c;
                    rcvState = StateReception.Payload;
                    break;

                case StateReception.Payload:
                    if(msgDecodedPayloadLength < 256) { 
                        msgDecodedPayload[msgDecodedPayloadIndex] = c;
                        msgDecodedPayloadIndex++;

                        if (msgDecodedPayloadIndex > msgDecodedPayloadLength-1)
                        {
                            rcvState = StateReception.CheckSum;
                            msgDecodedPayloadIndex = 0;
                        }
                    }
                    else
                    {
                        rcvState = StateReception.Waiting;
                    }
                    break;

                case StateReception.CheckSum:
                    byte receivedChecksum = c;
                    byte calculatedChecksum = CalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);

                    if (calculatedChecksum == receivedChecksum || msgDecodedFunction == 257)
                    {
                        ProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
                    }
                    rcvState = StateReception.Waiting;
                    break;

                default:
                    rcvState = StateReception.Waiting;
                    break;
            }
        }

        private void ProcessDecodedMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {

            switch (msgFunction)
            {
                case 0x0080:
                    string payloadString = Encoding.ASCII.GetString(msgPayload);
                    for (int i = 0; i < msgPayloadLength; i++)
                        robot.byteListReceived.Enqueue(msgPayload[i]);
                    break;

                case 0x0020:
                    int Numled = (int)(msgPayload[0]);

                    if (Numled == 0)
                    {   
                        EtatLEDVerte = Convert.ToBoolean(msgPayload[1]);
                    }
                    else if(Numled == 1)
                    {
                        EtatLEDRouge = Convert.ToBoolean(msgPayload[1]);
                    }
                    else if(Numled == 2)
                    {
                        EtatLEDOrange = Convert.ToBoolean(msgPayload[1]);
                    }
                    else if(Numled == 3)
                    {
                        EtatLEDBleue = Convert.ToBoolean(msgPayload[1]);
                    }
                    else if(Numled == 4)
                    {
                        EtatLEDBlanche = Convert.ToBoolean(msgPayload[1]);
                    }
                    


                break;

                case 0x0030:
                    if (msgPayloadLength == 5)
                    {
                         distanceIRExtremeDroite = (int)(msgPayload[0]);
                         distanceIRExtremeGauche = (int)(msgPayload[4]);
                         distanceIRGauche = (int)(msgPayload[3]);
                         distanceIRMilieu = (int)(msgPayload[2]);
                         distanceIRDroite = (int)(msgPayload[1]);
                    }
                    break;

                case 0x0040:

                    break;


                case 0x0050:
                    instant = (((int)msgPayload[1]) << 24) + (((int)msgPayload[2]) << 16) + (((int)msgPayload[3]) << 8) + ((int)msgPayload[4]);
                    robotState = (int)msgPayload[0];   
                    break;

                case 0x0061:
                    robot.positionXOdo = BitConverter.ToSingle(msgPayload, 4);
                    robot.positionYOdo = BitConverter.ToSingle(msgPayload, 8);
                    robot.angleRadian = BitConverter.ToSingle(msgPayload, 12);
                    robot.vitesseLineaire = BitConverter.ToSingle(msgPayload, 16);
                    robot.vitesseAngulaire = BitConverter.ToSingle(msgPayload, 20);
                    robot.vitesseDroit = BitConverter.ToSingle(msgPayload, 24);
                    robot.vitesseGauche = BitConverter.ToSingle(msgPayload, 28);
                    robot.time = BitConverter.ToUInt32(msgPayload, 32);


                    break;

                case 0x0070:

                    if (msgPayload[48] == 0)
                    {
                        PIDLineaire.Kp = BitConverter.ToDouble(msgPayload, 0);
                        PIDLineaire.ErreurPMax = BitConverter.ToDouble(msgPayload, 8);
                        PIDLineaire.Ki = BitConverter.ToDouble(msgPayload, 16);
                        PIDLineaire.ErreurIMax = BitConverter.ToDouble(msgPayload, 24);
                        PIDLineaire.Kd = BitConverter.ToDouble(msgPayload, 32);
                        PIDLineaire.ErreurDMax = BitConverter.ToDouble(msgPayload, 40);
                    }
                    else
                    {
                        PIDAngulaire.Kp = BitConverter.ToDouble(msgPayload, 0);
                        PIDAngulaire.ErreurPMax = BitConverter.ToDouble(msgPayload, 8);
                        PIDAngulaire.Ki = BitConverter.ToDouble(msgPayload, 16);
                        PIDAngulaire.ErreurIMax = BitConverter.ToDouble(msgPayload, 24);
                        PIDAngulaire.Kd = BitConverter.ToDouble(msgPayload, 32);
                        PIDAngulaire.ErreurDMax = BitConverter.ToDouble(msgPayload, 40);
                    }
                    
                    break;

                case 0x0075:
                    PIDLineaire.Erreur = BitConverter.ToDouble(msgPayload, 0);
                    PIDAngulaire.Erreur = BitConverter.ToDouble(msgPayload, 8);
                    PIDLineaire.ConsigneDebug = BitConverter.ToDouble(msgPayload, 16);
                    PIDAngulaire.ConsigneDebug = BitConverter.ToDouble(msgPayload, 24);
                    PIDLineaire.CorrP = BitConverter.ToDouble(msgPayload, 32);

                    break;


                case 0x0076:
                    PIDAngulaire.CorrP = BitConverter.ToDouble(msgPayload, 0);
                    PIDLineaire.CorrI = BitConverter.ToDouble(msgPayload, 8);
                    PIDAngulaire.CorrI = BitConverter.ToDouble(msgPayload, 16);
                    PIDLineaire.CorrD = BitConverter.ToDouble(msgPayload, 24);
                    PIDAngulaire.CorrD = BitConverter.ToDouble(msgPayload, 32);

                    break;

                case 0x0101:
                    double angleRad = BitConverter.ToDouble(msgPayload, 0);
                    double angleDeg = angleRad * (180.0 / Math.PI);
                    robot.angleActuelGhost = angleRad;
                    robot.positionXGhost = (float)BitConverter.ToDouble(msgPayload, 8);
                    robot.positionYGhost = (float)BitConverter.ToDouble(msgPayload, 16);
                    break;



            }
        }

        private void checkBoxLEDVerte_Checked(object sender, RoutedEventArgs e)
        {
            EtatLEDVerte = !EtatLEDVerte;
            byte[] array = new byte[2];
            array[0] = 0;
            array[1] = Convert.ToByte(EtatLEDVerte);
            UartEncodeAndSendMessage(0x0020, 2, array);
        }

        private void checkBoxLEDRouge_Checked(object sender, RoutedEventArgs e)
        {
            EtatLEDRouge = !EtatLEDRouge;
            byte[] array = new byte[2];
            array[0] = 1;
            array[1] = Convert.ToByte(EtatLEDRouge);
            UartEncodeAndSendMessage(0x0020, 2, array);
        }

        private void checkBoxLEDOrange_Checked(object sender, RoutedEventArgs e)
        {
            EtatLEDOrange = !EtatLEDOrange;
            byte[] array = new byte[2];
            array[0] = 2;
            array[1] = Convert.ToByte(EtatLEDOrange);
            UartEncodeAndSendMessage(0x0020, 2, array);
        }

        private void checkBoxLEDBleue_Checked(object sender, RoutedEventArgs e)
        {
            EtatLEDBleue = !EtatLEDBleue;
            byte[] array = new byte[2];
            array[0] = 3;
            array[1] = Convert.ToByte(EtatLEDBleue);
            UartEncodeAndSendMessage(0x0020, 2, array);
        }

        private void checkBoxLEDBlanche_Checked(object sender, RoutedEventArgs e)
        {
            EtatLEDBlanche = !EtatLEDBlanche;
            byte[] array = new byte[2];
            array[0] = 4;
            array[1] = Convert.ToByte(EtatLEDBlanche);
            UartEncodeAndSendMessage(0x0020, 2, array);
        }


        private void ApplyButton_Click(object sender, RoutedEventArgs e)
        {
            byte[] SetupAsserv = new byte[48];
            UartEncodeAndSendMessage(0x0072, 48, SetupAsserv);
        }

        private void ConsigneButton_Click(object sender, RoutedEventArgs e)
        {
            
        }

        private void Reset_Click(object sender, RoutedEventArgs e)
        {
            byte[] Consigne = new byte[16];
            PIDAngulaire.Consigne = 0;
            PIDLineaire.Consigne = 0;

            UartEncodeAndSendMessage(0x0071, 16, Consigne);
        }

        private void InitPID_Click(object sender, RoutedEventArgs e)
        {
            byte[] message = new byte[96];

            // ENVOIE DES COEFFICIENTS DU PID X
            byte[] valeur = BitConverter.GetBytes(PIDLineaire.Kp);
            Array.Copy(valeur, 0, message, 0, valeur.Length);
            valeur = BitConverter.GetBytes(PIDLineaire.Ki);
            Array.Copy(valeur, 0, message, 8, valeur.Length);
            valeur = BitConverter.GetBytes(PIDLineaire.Kd);
            Array.Copy(valeur, 0, message, 16, valeur.Length);
            valeur = BitConverter.GetBytes(PIDLineaire.ErreurPMax);
            Array.Copy(valeur, 0, message, 24, valeur.Length);
            valeur = BitConverter.GetBytes(PIDLineaire.ErreurIMax);
            Array.Copy(valeur, 0, message, 32, valeur.Length);
            valeur = BitConverter.GetBytes(PIDLineaire.ErreurDMax);
            Array.Copy(valeur, 0, message, 40, valeur.Length);


            //  ENVOIE DES COEFFICIENTS DU PID THETA
            valeur = BitConverter.GetBytes(PIDAngulaire.Kp);
            Array.Copy(valeur, 0, message, 48, valeur.Length);
            valeur = BitConverter.GetBytes(PIDAngulaire.Ki);
            Array.Copy(valeur, 0, message, 56, valeur.Length);
            valeur = BitConverter.GetBytes(PIDAngulaire.Kd);
            Array.Copy(valeur, 0, message, 64, valeur.Length);
            valeur = BitConverter.GetBytes(PIDAngulaire.ErreurPMax);
            Array.Copy(valeur, 0, message, 72, valeur.Length);
            valeur = BitConverter.GetBytes(PIDAngulaire.ErreurIMax);
            Array.Copy(valeur, 0, message, 80, valeur.Length);
            valeur = BitConverter.GetBytes(PIDAngulaire.ErreurDMax);
            Array.Copy(valeur, 0, message, 88, valeur.Length);

            UartEncodeAndSendMessage(0x0072, 96, message);
        }

        private void Test1_Click(object sender, RoutedEventArgs e)
        {
            byte[] message = new byte[16];

            PIDLineaire.Consigne = 0;
            PIDAngulaire.Consigne = 0.0;
            byte[] bytesDouble = BitConverter.GetBytes(PIDAngulaire.Consigne);
            Array.Copy(bytesDouble, 0, message, 8, bytesDouble.Length);

            UartEncodeAndSendMessage(0x0071, 16, message);
        }

        private void Test2_Click(object sender, RoutedEventArgs e)
        {
            byte[] message = new byte[16];

            PIDLineaire.Consigne = 0.0;
            PIDAngulaire.Consigne = 0;
            byte[] bytesDouble = BitConverter.GetBytes(PIDLineaire.Consigne);
            Array.Copy(bytesDouble, 0, message, 0, bytesDouble.Length);

            UartEncodeAndSendMessage(0x0071, 16, message);
        }

        private void Test3_Click(object sender, RoutedEventArgs e)
        {
            byte[] message = new byte[16];

            PIDLineaire.Consigne = 0.0;
            byte[] bytesDouble = BitConverter.GetBytes(PIDLineaire.Consigne);
            Array.Copy(bytesDouble, 0, message, 0, bytesDouble.Length);

            PIDAngulaire.Consigne = 0.0;
            byte[] bytesDouble2 = BitConverter.GetBytes(PIDAngulaire.Consigne);
            Array.Copy(bytesDouble2, 0, message, 8, bytesDouble2.Length);

            UartEncodeAndSendMessage(0x0071, 16, message);
        }


        public void CalculateAngleRestant()
        {
            
            if(worldMap.xDataValue != 0 || worldMap.yDataValue != 0)
            {
                byte[] message = new byte[8];

                robot.positionXCible = robot.positionXGhost - 20;
                robot.positionYCible = robot.positionYGhost - 50;
                robot.angleRadianCible = Math.Atan2(robot.positionYCible, robot.positionXCible);

                byte[] valeur = BitConverter.GetBytes((float)robot.positionXCible);
                Array.Copy(valeur, 0, message, 0, valeur.Length);
                valeur = BitConverter.GetBytes((float)robot.positionYCible);
                Array.Copy(valeur, 4, message, 4, valeur.Length);
                valeur = BitConverter.GetBytes(((float)robot.angleRadianCible));
                Array.Copy(valeur, 8, message, 8, valeur.Length);

                UartEncodeAndSendMessage(0x0100, 12, message);

                robot.angleCible = ((450 - (robot.angleRadianCible * 360.0 / (2 * Math.PI))) % 360);
                worldMap.xDataValue = 0;
                worldMap.yDataValue = 0;

            }
            }


    } 
}
