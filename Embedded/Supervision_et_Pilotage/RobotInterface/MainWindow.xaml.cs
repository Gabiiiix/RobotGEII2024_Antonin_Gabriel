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

        Robot robot = new Robot();


        public MainWindow()
        {
            InitializeComponent();

            serialPort1 = new ExtendedSerialPort("COM7", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();

            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();

        }

        private void TimerAffichage_Tick(object sender, EventArgs e)
        {
            while (robot.byteListReceived.Count != 0)
            {
                TextBoxReception.Text += robot.byteListReceived.Dequeue().ToString("X2") + " ";
            }

            textboxCapteurDroite.Text = "IR Droit: " + Convert.ToString(distanceIRDroite) + "cm";
            textboxCapteurGauche.Text = "IR Gauche: " + Convert.ToString(distanceIRGauche) + "cm";
            textboxCapteurMilieu.Text = "IR Centre: " + Convert.ToString(distanceIRMilieu) + "cm";
            textboxCapteurDroiteExtreme.Text = "IR Droite Extreme: " + Convert.ToString(distanceIRExtremeDroite) + "cm";
            textboxCapteurGaucheExtreme.Text = "IR Gauche Extreme: " + Convert.ToString(distanceIRExtremeGauche) + "cm" ;

            textboxRobotState.Text = "Robot␣State :\n" + ((StateRobot)(robotState)).ToString();
            textboxRobotStateTimer.Text = "Time:\n" + instant.ToString() + " ms";

            checkBoxLEDRouge.IsChecked = EtatLEDRouge;
            checkBoxLEDVerte.IsChecked = EtatLEDVerte;
            checkBoxLEDBlanche.IsChecked = EtatLEDBlanche;
            checkBoxLEDBleue.IsChecked = EtatLEDBleue;
            checkBoxLEDOrange.IsChecked = EtatLEDOrange;
        }

        private void boutonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            SendMessage();
        }

        private void TextBoxEmission_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                SendMessage();
            }
        }

        public void SendMessage()
        {
            serialPort1.WriteLine(TextBoxEmission.Text);
            TextBoxEmission.Text = "";
        }


        private void boutonClear_Click(object sender, RoutedEventArgs e)
        {
            TextBoxReception.Text = "";
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

                    if (calculatedChecksum == receivedChecksum)
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
    }
}
