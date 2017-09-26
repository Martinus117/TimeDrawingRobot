using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DrawingMachine
{

    public partial class Form1 : Form
    {
        public string badPoints;
        public Graphics g;
        const Double lInner = 450;
        const Double lOuter = 540;
        const Double pWidth = 900;
        const Double pHeight = 530;
        
        const int rX = 150;
        const int rY = 10;
        const float cFactor = 5.6F;
        PointF zPoint = new PointF(450, -250);  // from mPoint
        PointF mPoint = new PointF(150, 540); // from WindowsCoordinates
        Double iMax = 0, iMin=360, oMax=0, oMin=360;
        // int maxP = 0;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.Width = 1500;
            this.Height = 1000;
            g = this.CreateGraphics();
            g.Clear(Color.White);
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            DrawBackground();       
        }

        private void Button2_Click(object sender, EventArgs e)
        {
            DrawPoint(0, 0);
            DrawPoint(0, pHeight);
            DrawPoint(pWidth, pHeight);
            DrawPoint(pWidth, 0);
        }

        private void DrawPoint(double xx, double yy)
        {
            PointF t1 = new PointF((float)xx, (float)yy);
            double oAngleRad = 0, iAngleRad = 0;
            InnerOuterAngles(t1, ref iAngleRad, ref oAngleRad);

            double oAngle = Rad2Deg(oAngleRad);

            double iAngle = Rad2Deg(iAngleRad);
            textBox2.Text += "x=" + (t1.X.ToString("000") + "  y=" + t1.Y.ToString("000") + " I:" + iAngle.ToString("###0.00") + " O:" + oAngle.ToString("###0.00") + Environment.NewLine);
            DrawParts(t1, Color.Purple);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            int iSleep = 150; // 50
            int iStep = 25;  // 10
            for (int ii = 0; ii <= pHeight; ii += iStep)
            {
                System.Threading.Thread.Sleep(iSleep);
                this.Show();
                Color c = new Color();
                c = Color.FromArgb(10, 10, 10);
                DrawParts(new PointF(0,ii), c);
            }
          
            for (int ii = 0; ii <= pWidth; ii += iStep)
            {
                System.Threading.Thread.Sleep(iSleep);
                this.Show();
                DrawParts(new PointF(ii, 530), Color.Blue);
            }

            for (int ii = (int)pHeight; ii >= 0; ii -= iStep)
            {
                System.Threading.Thread.Sleep(iSleep);
                this.Show();
                DrawParts(new PointF((float)pWidth, ii), Color.Green);
            }

            for (int ii = (int)pWidth; ii >= 0; ii -= iStep)
            {
                System.Threading.Thread.Sleep(iSleep);
                this.Show();
                DrawParts(new PointF(ii, 0), Color.Gold);
            }
            textBox2.Text += "I_Max:" + iMax.ToString("###0.00") + "I_Min:" + iMin.ToString("###0.00") + Environment.NewLine;
            textBox2.Text += "O_Max:" + oMax.ToString("###0.00") + "O_Min:" + oMin.ToString("###0.00") + Environment.NewLine;

        }

        private void button4_Click(object sender, EventArgs e)
        {
            g.Clear(Color.White);
            label2.Text = "";
            DrawBackground();
            Double maxL = Math.Sqrt(Math.Pow(zPoint.X, 2) + Math.Pow((pHeight + Math.Abs(zPoint.Y)), 2));
            textBox2.Text = "Max L: " + maxL.ToString();
        }


        private void DrawBackground()
        {
            Pen p = new Pen(Color.Blue, 1);
            Rectangle rect = new Rectangle(rX, rY, (int)pWidth, (int)pHeight);
            p.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
            g.DrawRectangle(p, rect);
            DrawPoint(zPoint);
            DrawPoint(0, 0);
            DrawPoint(900, 530);
            DrawPoint(900, 0);
            DrawPoint(0, 530);
            DrawPoint(100, 100);
            DrawPoint(10, 80);
            DrawPoint(12, 80);
        }

        private void DrawParts(PointF pf, Color color)
        {
            double oAngleRad=0, iAngleRad=0;
            InnerOuterAngles(pf, ref iAngleRad, ref oAngleRad);           
            
            double lx = lInner * Math.Sin(iAngleRad);
            double ly = lInner * Math.Cos(iAngleRad);
            double txi = zPoint.X - lx;
            double tyi = zPoint.Y - ly;

            if (checkBox1.Checked)
            {
                g.DrawLine(new Pen(System.Drawing.Color.Yellow, 2), M2Win(zPoint), M2Win(new PointF((float)txi, (float)tyi)));
                Point iPoint = M2Win(new PointF((float)txi, (float)tyi));
                g.DrawEllipse(new Pen(color, 2), iPoint.X - 2, iPoint.Y - 2, 4, 4);
            }

            double ax = oAngleRad - iAngleRad;
            lx = lOuter * Math.Sin(ax);
            ly = lOuter * Math.Cos(ax);
            double txo = txi - lx;
            double tyo = tyi + ly;

            double iAngle = Rad2Deg(iAngleRad);
            double oAngle = Rad2Deg(oAngleRad);
            if (iAngle > iMax)
                iMax = iAngle;
            if (iAngle < iMin)
                iMin = iAngle;
            if (oAngle > oMax)
                oMax = oAngle;
            if (oAngle < oMin)
                oMin = oAngle;

            Point oPoint = M2Win(new PointF((float)txo, (float)tyo));
            g.DrawEllipse(new Pen(color, 1), oPoint.X - 1, oPoint.Y - 1, 2, 2);

            if (checkBox1.Checked)
            {
                g.DrawLine(new Pen(color, 2), M2Win(new PointF((float)txi, (float)tyi)), M2Win(new PointF((float)txo, (float)tyo)));
            }
       }

        private void InnerOuterAngles(PointF pf, ref Double iAngle, ref Double oAngle)
        {
            Double xx, yy, zz;
            // li und lo unterschiedlich
            // Hypothenuse z berechnen aus x, y (z = SQRT(x2 + y2)
            // x: Fallunterscheidung
            // Cosinus-Satz: c2 = a2 + b2 -2abcos(y)
            if (pf.X <= zPoint.X)
                xx = zPoint.X - pf.X;
            else
                xx = pf.X - zPoint.X;
            yy = pf.Y + Math.Abs(zPoint.Y);
            zz = Math.Sqrt(xx * xx + yy * yy);

            // Outer Angle
            oAngle = Math.Acos((lInner * lInner + lOuter * lOuter - zz * zz) / (2 * lInner * lOuter));
            
            // inner Angle
            Double alpha1 = Math.Acos((zz * zz + lInner * lInner - lOuter * lOuter) / (2 * lInner * zz));
            Double alpha2 = Math.Asin(xx / zz);
            if (pf.X < zPoint.X)
                iAngle = Math.PI - alpha1 - alpha2;
            else
                iAngle = Math.PI + alpha2 - alpha1;
        }

        private double Rad2Deg(double rad)
        {            
            return rad * (180.0 / Math.PI);            
        }

        private void DrawPoint(PointF p)
        {
            Point pt = M2Win(p);
            Pen pen = new Pen(System.Drawing.Color.Red, 1);
            g.DrawEllipse(pen, pt.X - 2, pt.Y - 2, 4, 4);
        }

        private void DrawPoint(int x, int y)
        {
            DrawPoint(new PointF(x, y));
        }

        private Point M2Win(PointF p)
        {
            return new Point((int)(p.X + mPoint.X), (int)(mPoint.Y - p.Y));
        }

        private PointF Win2M(PointF p)
        {
            return new PointF(mPoint.X - p.X, p.Y - mPoint.Y);
        }


        private GraphicsPath PrepLetter(string letter)
        {
            // FontFamily ff = new FontFamily("1CamBam_Stick_2");
            // FontFamily ff = new FontFamily("Kunstler Script");
            // FontFamily ff = new FontFamily("Arial");
            FontFamily ff = new FontFamily("Kristen ITC");
            GraphicsPath gp = new System.Drawing.Drawing2D.GraphicsPath();
            gp.AddString(letter, ff, (int)FontStyle.Regular, 350, new Point(0, 0), StringFormat.GenericTypographic);
            return gp;
        }

        private void DrawPath(GraphicsPath gp)
        {
            int effPoints = 0;
            PointF p1, p2;
            float factor = 1.0F;
            Pen pen = new Pen(Color.Red, 2);
            Color lastColor = pen.Color;
            // dispPoint(0, gp.PathPoints[0], gp.PathTypes[0], pen.Color);
            for (int ii = 0; ii < gp.PointCount; ii++)
            {
                int ll = 1;                
                String iiString = ";" + ii.ToString();
                if (badPoints.IndexOf(iiString) > 0 && ii < gp.PointCount - 1)
                {
                    ii++;
                    ll++;
                }

                if (ii > 0)
                {
                    effPoints++;
                    p1 = gp.PathPoints[ii - ll];
                    p2 = gp.PathPoints[ii];
                    p1.Y = p1.Y * factor;
                    p1.Y = mPoint.Y - p1.Y;
                    p2.Y = p2.Y * factor;
                    p2.Y = mPoint.Y - p2.Y;
                    p1.X = p1.X * factor;
                    p2.X = p2.X * factor;

                    if (gp.PathTypes[ii] == 0)
                    {
                        lastColor = pen.Color;
                        pen.Color = Color.Yellow;
                    }
                    else if (gp.PathTypes[ii - ll] == 0)
                    {
                        if (lastColor == Color.Red) pen.Color = Color.Blue;
                        else pen.Color = Color.Red;
                    }
                    DrawParts(p1, pen.Color);
                    System.Threading.Thread.Sleep(10);

                    if (ii + 1 == gp.PointCount)
                        DrawParts(p2, pen.Color);
                }
            }
            label2.Text = gp.PointCount.ToString() + " : " + effPoints.ToString();
        }

        private void DrawPathLine(GraphicsPath gp)
        {
            PointF p1, p2;
            float factor = 3.5F;
            factor = 1.0F;
            Pen pen = new Pen(Color.Red, 2);
            Color lastColor = pen.Color;
            // ;xx;yy
            int effPoints = 0; 
            // dispPoint(0, gp.PathPoints[0], gp.PathTypes[0], pen.Color);
            for (int ii = 0; ii < gp.PointCount; ii++)
            {
                int ll = 1;
                String iiString = ";" + ii.ToString();
                if (badPoints.IndexOf(iiString) > 0) { 
                    ii++;
                    ll++;
                }

                if (ii > 0)
                {
                    effPoints++;
                    p1 = gp.PathPoints[ii - ll];
                    p2 = gp.PathPoints[ii];
                    p1.Y = p1.Y * factor;
                    p1.Y = mPoint.Y - p1.Y;
                    p2.Y = p2.Y * factor;
                    p2.Y = mPoint.Y - p2.Y;
                    p1.X = p1.X * factor;
                    p2.X = p2.X * factor;
                    if (gp.PathTypes[ii] == 0)
                    {
                        lastColor = pen.Color;
                        pen.Color = Color.Yellow;
                    }
                    else if (gp.PathTypes[ii - ll] == 0)
                    {
                        if (lastColor == Color.Red) pen.Color = Color.Blue;
                        else pen.Color = Color.Red;
                    }
                    g.DrawLine(pen, M2Win(p1), M2Win(p2));
                    System.Threading.Thread.Sleep(20);
                }
            }
            label2.Text = gp.PointCount.ToString() + " : " + effPoints.ToString();
            textBox2.Refresh();
        }

        private void button5_Click(object sender, EventArgs e)
        { 
            GraphicsPath gp = PrepLetter(textBox1.Text);
            testSmall(gp);
            // (gp.Flatten();        
            DrawPath(gp);
            //DrawPathLine(gp);
            SolidBrush brush = new SolidBrush(Color.Black);

        }

        private void writeGp(GraphicsPath g)
        {
            textBox2.Text = "double Matrix[][2] = { \r\n";
            for (int ii=0;ii<g.PointCount; ii++)
            {
                String iiString = ";" + ii.ToString();
                if (!(badPoints.IndexOf(iiString) > 0))
                {
                    textBox2.Text += ii.ToString() 
                        +  "= {" + g.PathPoints[ii].X.ToString("0.00") 
                        + ", " + g.PathPoints[ii].Y.ToString("0.00") + "},  " 
                        + g.PathTypes[ii].ToString() +  "\r\n";
                }
            }
            textBox2.Text += "};";
        }

        private void writeGpSmall(GraphicsPath g)
        {
            float x, y;
            textBox2.Text = "";
            float maxY = 0;
            for (int ii = 0; ii < g.PointCount; ii++)
            {
                if (g.PathPoints[ii].Y > maxY)
                    maxY = g.PathPoints[ii].Y;
            }
            textBox2.Text = "maxY: " + maxY.ToString() + Environment.NewLine;
            
            for (int ii = 0; ii < g.PointCount; ii++)
            {
                 String iiString = ";" + ii.ToString();
                if (!(badPoints.IndexOf(iiString) > 0))
                {
                    x = (g.PathPoints[ii].X) / cFactor;
                    y = (450 - g.PathPoints[ii].Y) / cFactor;
                    textBox2.Text += x.ToString("0.00") + "," 
                        + y.ToString("0.00") + "," +                        
                        (g.PathTypes[ii] != 1 ? g.PathTypes[ii].ToString() + ";" : ";");
                }
            }
        }

        private void testSmall(GraphicsPath _gp)
        {
            badPoints = "";
            _gp.Flatten();
            int count = _gp.PointCount;
            PointF pf;
            pf = _gp.PathPoints[0];

            for (int i = 1; i < count; i++)
            {
                if ((Math.Abs(_gp.PathPoints[i].X - pf.X) < 2
                     && Math.Abs(_gp.PathPoints[i].Y - pf.Y) < 2)    // 1
                     && _gp.PathTypes[i] == 1)
                {
                    badPoints += ";" + i.ToString();
                    i++;
                }
                else
                    pf = _gp.PathPoints[i];
            }
        }

        private void button6_Click_1(object sender, EventArgs e)
        {
            GraphicsPath gp = PrepLetter(textBox1.Text);
            testSmall(gp);
            writeGp(gp);
            DrawPathLine(gp);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            GraphicsPath gp = PrepLetter(textBox1.Text);
            testSmall(gp);
            writeGpSmall(gp);
        }
    }
}