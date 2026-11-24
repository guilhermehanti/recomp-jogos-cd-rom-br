using System;
using System.Drawing;
using System.IO;
using System.Media;
using System.Windows.Forms;

sealed class DrivingPreview : Form
{
    static readonly Rectangle Cab = Rectangle.FromLTRB(263, 137, 382, 291);
    readonly Bitmap workshop;
    readonly Bitmap dashboard, left, sky, right, skyline;
    readonly Bitmap[] wheels;
    readonly Bitmap[] curtains;
    readonly SoundPlayer[] sounds;
    int radio;
    readonly Timer clock = new Timer { Interval = 16 };
    int curtainFrame, curtainTime;
    bool enteringCab;
    double speed, lateral, distance;
    bool accelerating, braking, leftHeld, rightHeld;
    bool driving;
    int steering;
    int dragStart;

    DrivingPreview(Bitmap workshop, Bitmap[] assets, Bitmap[] curtains, SoundPlayer[] sounds)
    {
        this.workshop = workshop;
        this.curtains = curtains;
        this.sounds = sounds;
        dashboard = assets[0];
        left = assets[1];
        sky = assets[2];
        right = assets[3];
        skyline = assets[4];
        wheels = new[] { assets[6], assets[5], assets[7] };
        ClientSize = new Size(640, 480);
        FormBorderStyle = FormBorderStyle.FixedSingle;
        MaximizeBox = false;
        DoubleBuffered = true;
        UpdateScene(false);
        clock.Tick += (sender, args) => Advance(0.016);
        Shown += (sender, args) => clock.Start();
        Deactivate += (sender, args) => ClearInput();
    }

    void UpdateScene(bool next)
    {
        if (driving)
            sounds[radio].Stop();
        radio = 0;
        Capture = false;
        Cursor = Cursors.Default;
        driving = next;
        steering = 0;
        ClearInput();
        speed = lateral = distance = 0;
        curtainFrame = curtainTime = 0;
        enteringCab = false;
        Text = driving ? "Big Job BR | Setas: dirigir | B: buzina | R: rádio | Esc: oficina"
                       : "Big Job BR | Oficina | Clique na cabine ao centro";
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        Graphics g = e.Graphics;
        if (!driving)
        {
            g.DrawImageUnscaled(workshop, 0, 0);
            g.DrawImageUnscaled(curtains[curtainFrame], 210, 119);
            return;
        }
        g.Clear(Color.FromArgb(63, 62, 60));
        g.DrawImageUnscaled(sky, 16, 0);
        g.DrawImageUnscaled(skyline, 16, 65);
        DrawRoad(g);
        g.DrawImageUnscaled(dashboard, 16, 265);
        g.DrawImageUnscaled(left, 0, 0);
        g.DrawImageUnscaled(right, 616, 0);
        g.DrawImageUnscaled(wheels[steering + 1], 25, 265);
    }

    void DrawRoad(Graphics g)
    {
        float center = (float)(316 - lateral * 120);
        g.FillPolygon(Brushes.DimGray, new[] {
            new PointF(306, 181), new PointF(326, 181),
            new PointF(center + 255, 265), new PointF(center - 255, 265)
        });
        for (int i = 0; i < 8; ++i)
        {
            float depth = (float)((i + distance) % 8 / 8);
            float perspective = depth * depth;
            float x = 316 + (center - 316) * perspective;
            g.FillRectangle(Brushes.Khaki, x - 1 - 4 * perspective,
                            181 + 84 * perspective, 2 + 8 * perspective, 1 + 9 * perspective);
        }
    }

    void ClearInput()
    {
        accelerating = braking = leftHeld = rightHeld = false;
        steering = 0;
    }

    void Advance(double seconds)
    {
        if (!driving)
        {
            if (enteringCab)
            {
                curtainTime += (int)Math.Round(seconds * 1000);
                if (curtainTime >= curtains.Length * 50)
                {
                    UpdateScene(true);
                    return;
                }
                curtainFrame = Math.Min(curtains.Length - 1, curtainTime / 50);
                Invalidate();
            }
            return;
        }
        if (!Capture)
            steering = (rightHeld ? 1 : 0) - (leftHeld ? 1 : 0);
        double change = braking ? -1.8 : accelerating ? 0.7 : -0.23;
        speed = Math.Max(0, Math.Min(1, speed + change * seconds));
        lateral = Math.Max(-1, Math.Min(1, lateral + steering * speed * seconds));
        distance = (distance + speed * seconds * 7) % 8;
        Invalidate();
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
            clock.Dispose();
        base.Dispose(disposing);
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        if (e.Button != MouseButtons.Left)
            return;
        if (!driving && !enteringCab && Cab.Contains(e.Location))
        {
            enteringCab = true;
            Cursor = Cursors.Default;
        }
        else if (driving && new Rectangle(25, 265, 213, 213).Contains(e.Location))
        {
            dragStart = e.X;
            Capture = true;
        }
    }

    void Steer(int direction)
    {
        steering = direction;
        Invalidate();
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        if (!driving)
            Cursor = !enteringCab && Cab.Contains(e.Location) ? Cursors.Hand : Cursors.Default;
        else if (Capture)
            Steer(Math.Abs(e.X - dragStart) < 5 ? 0 : Math.Sign(e.X - dragStart));
    }

    protected override void OnMouseCaptureChanged(EventArgs e)
    {
        base.OnMouseCaptureChanged(e);
        if (driving && !Capture)
            Steer(0);
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        if (Capture && e.Button == MouseButtons.Left)
            Capture = false;
    }

    protected override bool IsInputKey(Keys key)
    {
        return key == Keys.Up || key == Keys.Down || key == Keys.Left ||
               key == Keys.Right || base.IsInputKey(key);
    }

    protected override void OnKeyDown(KeyEventArgs e)
    {
        SetKey(e.KeyCode, true);
    }

    protected override void OnKeyUp(KeyEventArgs e)
    {
        SetKey(e.KeyCode, false);
    }

    void SetKey(Keys key, bool pressed)
    {
        if (!driving)
            return;
        if (key == Keys.Escape && pressed)
            UpdateScene(false);
        else if (key == Keys.Left)
            leftHeld = pressed;
        else if (key == Keys.Right)
            rightHeld = pressed;
        else if (key == Keys.Up)
            accelerating = pressed;
        else if (key == Keys.Down)
            braking = pressed;
        else if (key == Keys.B && !pressed)
        {
            sounds[radio].Stop();
            radio = 0;
            sounds[0].Play();
        }
        else if (key == Keys.R && !pressed)
        {
            sounds[radio].Stop();
            radio = (radio + 1) % sounds.Length;
            if (radio != 0)
                sounds[radio].PlayLooping();
        }
    }

    [STAThread]
    static int Main(string[] args)
    {
        string analysis = Path.GetFullPath(Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory, "..", "analysis"));
        bool verify = args.Length == 1 && args[0] == "--verify";
        var assets = new Bitmap[8];
        var curtains = new Bitmap[6];
        var sounds = new SoundPlayer[4];
        try
        {
            int[] ids = { 279, 281, 282, 286, 319, 347, 348, 353 };
            for (int i = 0; i < ids.Length; ++i)
                assets[i] = new Bitmap(Path.Combine(analysis, "driving",
                    String.Format("BIGJOBHD-{0:000}.png", ids[i])));
            for (int i = 0; i < curtains.Length; ++i)
                curtains[i] = new Bitmap(Path.Combine(i == 0 ? analysis :
                    Path.Combine(analysis, "curtains"),
                    String.Format("BIGJOBHD-{0:000}.png", i + 1)));
            string[] soundNames = { "horn.wav", "radio1.wav", "radio2.wav", "radio3.wav" };
            for (int i = 0; i < sounds.Length; ++i)
            {
                sounds[i] = new SoundPlayer(Path.Combine(analysis, "driving", soundNames[i]));
                sounds[i].Load();
            }
            using (var workshop = new Bitmap(Path.Combine(analysis, "BIGJOBHD-000.png")))
            using (var window = new DrivingPreview(workshop, assets, curtains, sounds))
            {
                if (verify)
                    window.Verify();
                else
                    Application.Run(window);
            }
            return 0;
        }
        catch (Exception error)
        {
            if (!verify)
                MessageBox.Show(error.Message, "Big Job BR");
            return 1;
        }
        finally
        {
            if (sounds[0] != null && sounds[0].IsLoadCompleted)
                sounds[0].Stop();
            foreach (SoundPlayer sound in sounds)
                if (sound != null)
                    sound.Dispose();
            foreach (Bitmap asset in assets)
                if (asset != null)
                    asset.Dispose();
            foreach (Bitmap curtain in curtains)
                if (curtain != null)
                    curtain.Dispose();
        }
    }

    void Verify()
    {
        Advance(1);
        if (enteringCab || curtainFrame != 0)
            throw new InvalidOperationException("A cortina abriu sem clique.");
        OnMouseDown(new MouseEventArgs(MouseButtons.Left, 1, 0, 0, 0));
        if (driving)
            throw new InvalidOperationException("Clique fora da cabine alterou a cena.");
        OnMouseDown(new MouseEventArgs(MouseButtons.Left, 1, 320, 220, 0));
        for (int i = 1; i < curtains.Length; ++i)
        {
            Advance(0.05);
            if (driving || curtainFrame != i)
                throw new InvalidOperationException("Quadro incorreto na entrada da cabine.");
        }
        Advance(0.05);
        if (!driving || steering != 0)
            throw new InvalidOperationException("A cabine não abriu a direção.");
        using (var image = new Bitmap(640, 480))
        using (var graphics = Graphics.FromImage(image))
        {
            var paint = new PaintEventArgs(graphics, ClientRectangle);
            OnPaint(paint);
            if (image.GetPixel(30, 10).ToArgb() != sky.GetPixel(14, 10).ToArgb())
                throw new InvalidOperationException("A cena de direção não foi desenhada.");
        }
        SetKey(Keys.Up, true);
        SetKey(Keys.Left, true);
        Advance(0.2);
        if (steering != -1 || speed <= 0 || lateral >= 0 || distance <= 0)
            throw new InvalidOperationException("O volante não virou à esquerda.");
        SetKey(Keys.Left, false);
        SetKey(Keys.Right, true);
        Advance(0.2);
        if (steering != 1)
            throw new InvalidOperationException("O volante não virou à direita.");
        SetKey(Keys.Up, false);
        SetKey(Keys.Down, true);
        Advance(0.2);
        if (speed != 0)
            throw new InvalidOperationException("O veículo não parou.");
        SetKey(Keys.Escape, true);
        Advance(1);
        if (driving || enteringCab || curtainFrame != 0)
            throw new InvalidOperationException("Esc não voltou à oficina.");
    }
}
