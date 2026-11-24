using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

sealed class CurtainPreview : Form
{
    readonly Bitmap[] scenes;
    readonly Timer playback = new Timer { Interval = 50 };
    int frame;

    CurtainPreview(Bitmap[] scenes)
    {
        this.scenes = scenes;
        ClientSize = new Size(640, 480);
        FormBorderStyle = FormBorderStyle.FixedSingle;
        MaximizeBox = false;
        DoubleBuffered = true;
        playback.Tick += (sender, args) =>
        {
            ShowFrame(frame + 1);
            if (frame == scenes.Length - 1)
                playback.Stop();
        };
        UpdateTitle();
    }

    void UpdateTitle()
    {
        Text = "Big Job BR | Cortina " + (frame + 1) + "/6 | Enter: animar | Setas: quadros";
    }

    void ShowFrame(int index)
    {
        frame = index;
        UpdateTitle();
        Invalidate();
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
            playback.Dispose();
        base.Dispose(disposing);
    }

    protected override bool ProcessCmdKey(ref Message message, Keys key)
    {
        switch (key)
        {
            case Keys.Enter:
                playback.Stop();
                ShowFrame(0);
                playback.Start();
                return true;
            case Keys.Left:
                playback.Stop();
                ShowFrame((frame + scenes.Length - 1) % scenes.Length);
                return true;
            case Keys.Right:
            case Keys.Space:
                playback.Stop();
                ShowFrame((frame + 1) % scenes.Length);
                return true;
            default: return base.ProcessCmdKey(ref message, key);
        }
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        e.Graphics.DrawImageUnscaled(scenes[frame], 0, 0);
    }

    [STAThread]
    static int Main(string[] args)
    {
        string analysis = Path.GetFullPath(Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory, "..", "analysis"));
        var scenes = new Bitmap[6];
        bool verify = args.Length == 1 && args[0] == "--verify";
        try
        {
            using (var background = new Bitmap(Path.Combine(analysis, "BIGJOBHD-000.png")))
            {
                if (background.Size != new Size(640, 480))
                    throw new InvalidDataException("Fundo fora do tamanho esperado.");
                for (int i = 0; i < scenes.Length; ++i)
                {
                    string folder = i == 0 ? analysis : Path.Combine(analysis, "curtains");
                    using (var curtain = new Bitmap(Path.Combine(folder,
                        String.Format("BIGJOBHD-{0:000}.png", i + 1))))
                    {
                        if (curtain.Size != new Size(211, 187))
                            throw new InvalidDataException("Quadro fora do tamanho esperado.");
                        scenes[i] = new Bitmap(640, 480);
                        using (var drawing = Graphics.FromImage(scenes[i]))
                        {
                            drawing.DrawImageUnscaled(background, 0, 0);
                            drawing.DrawImageUnscaled(curtain, 210, 119);
                        }
                        if (verify)
                            VerifyPixels(background, curtain, scenes[i]);
                    }
                }
            }
            if (!verify)
            {
                Application.EnableVisualStyles();
                using (var window = new CurtainPreview(scenes))
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
            foreach (var scene in scenes)
                if (scene != null)
                    scene.Dispose();
        }
    }

    static void VerifyPixels(Bitmap background, Bitmap curtain, Bitmap scene)
    {
        for (int y = 0; y < scene.Height; ++y)
        for (int x = 0; x < scene.Width; ++x)
        {
            int localX = x - 210, localY = y - 119;
            Color expected = background.GetPixel(x, y);
            if (localX >= 0 && localX < curtain.Width &&
                localY >= 0 && localY < curtain.Height)
            {
                Color pixel = curtain.GetPixel(localX, localY);
                if (pixel.A == 255)
                    expected = pixel;
                else if (pixel.A != 0)
                    throw new InvalidDataException("Alfa inesperado no quadro.");
            }
            if (expected.ToArgb() != scene.GetPixel(x, y).ToArgb())
                throw new InvalidDataException("Composição divergente em " + x + "," + y);
        }
    }
}
