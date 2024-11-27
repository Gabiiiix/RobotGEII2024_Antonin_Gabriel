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

            serialPort1 = new ExtendedSerialPort("COM3", 115200, Parity.None, 8, StopBits.One);
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

            serialPort1.Write(msg,0,msg.Length);
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
                        msgDecodedPayload[msgDecodedPayloadIndex] = c;
                        msgDecodedPayloadIndex++;

                        if (msgDecodedPayloadIndex > msgDecodedPayloadLength-1)
                        {
                            rcvState = StateReception.CheckSum;
                            msgDecodedPayloadIndex = 0;
                        }
                    break;

                case StateReception.CheckSum:
                    byte receivedChecksum = c;
                    byte calculatedChecksum = CalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);

                    if (calculatedChecksum == receivedChecksum)
                    {
                        ProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
                    }
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

                    // Code de deux octets, de manière:
                    //  E000 00XX XXXX XXXX,
                    // Si X = 1, alors la LED désigné (selon ça position) passe à l'état E
                    // Si X = 0, La LED n'est pas affecté
                break;

                case 0x0030:
                    if (msgPayloadLength == 5)
                    {
                        int DistanceTelemetreGaucheExtreme = (int)(msgPayload[0]);
                        int DistanceTelemetreGauche = (int)(msgPayload[1]);
                        int DistanceTelemetreCentre = (int)(msgPayload[2]);
                        int DistanceTelemetreDroit = (int)(msgPayload[3]);
                        int DistanceTelemetreDroitExtreme = (int)(msgPayload[4]);
                    }
                    break;

                case 0x0040:

                    break;
            }
        }
        

       
    }
}
