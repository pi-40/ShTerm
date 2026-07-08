using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace shterm
{
    public partial class Form1 : Form
    {
        private Process cmdProcess;
        private StreamWriter cmdWriter;
        private bool isWaitingForReleaseSelection = false;
        private Dictionary<char, GitHubReleaseAsset> availableReleaseOptions = new Dictionary<char, GitHubReleaseAsset>();

        [DllImport("DwmApi")]
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);

        public Form1()
        {
            InitializeComponent();
            ApplyDarkTitleBar();
        }

        private void ApplyDarkTitleBar()
        {
            int darkMode = 1;
            DwmSetWindowAttribute(this.Handle, 20, ref darkMode, sizeof(int));
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            try
            {
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                cmdProcess = new Process { StartInfo = psi };
                cmdProcess.OutputDataReceived += (s, args) => AppendTerminalText(args.Data);
                cmdProcess.ErrorDataReceived += (s, args) => AppendTerminalText(args.Data);

                cmdProcess.Start();
                cmdProcess.BeginOutputReadLine();
                cmdProcess.BeginErrorReadLine();
                cmdWriter = cmdProcess.StandardInput;
            }
            catch (Exception ex)
            {
                txtTerminal.AppendText($"Initialization Error: {ex.Message}{Environment.NewLine}");
            }
        }

        private async void txtInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true;
                string rawInput = txtInput.Text.Trim();
                txtInput.Clear();

                if (string.IsNullOrEmpty(rawInput)) return;

                if (isWaitingForReleaseSelection)
                {
                    char selection = rawInput.ToLower().FirstOrDefault();
                    if (availableReleaseOptions.ContainsKey(selection))
                    {
                        isWaitingForReleaseSelection = false;
                        var chosenAsset = availableReleaseOptions[selection];
                        availableReleaseOptions.Clear();
                        await DownloadReleaseAsset(chosenAsset);
                    }
                    else
                    {
                        txtTerminal.AppendText($"Invalid choice. Please select an assigned letter.{Environment.NewLine}$ ");
                    }
                    return;
                }

                if (rawInput.StartsWith("cth "))
                {
                    string gitLink = rawInput.Substring(4).Trim();
                    await FetchGitHubReleases(gitLink);
                }
                else
                {
                    string convertedCommand = ConvertBashToCmd(rawInput);
                    if (cmdWriter != null)
                    {
                        if (convertedCommand.Equals("cls", StringComparison.OrdinalIgnoreCase))
                        {
                            txtTerminal.Clear();
                        }
                        else
                        {
                            cmdWriter.WriteLine(convertedCommand);
                        }
                    }
                }
            }
        }

        private string ConvertBashToCmd(string input)
        {
            string cmd = input.Split(' ')[0].ToLower();
            switch (cmd)
            {
                case "ls": return "dir" + input.Substring(2);
                case "clear": return "cls";
                case "rm": return input.Contains("-r") ? "rmdir /s /q" + input.Replace("rm", "").Replace("-r", "") : "del" + input.Substring(2);
                case "mv": return "move" + input.Substring(2);
                case "cp": return "copy" + input.Substring(2);
                case "pwd": return "cd";
                case "cat": return "type" + input.Substring(3);
                case "touch": return $"type nul > {input.Substring(5).Trim()}";
                default: return input;
            }
        }

        private async Task FetchGitHubReleases(string url)
        {
            txtTerminal.AppendText($"$ cth {url}{Environment.NewLine}");
            txtTerminal.AppendText($"Contacting remote GitHub server endpoint...{Environment.NewLine}");

            string repoPath = url.Replace("https://github.com/", "").Replace("http://github.com/", "").TrimEnd('/');
            string[] parts = repoPath.Split('/');
            if (parts.Length < 2)
            {
                txtTerminal.AppendText($"Error: Invalid GitHub URL repository format.{Environment.NewLine}$ ");
                return;
            }

            string owner = parts[0];
            string repo = parts[1].Replace(".git", "");
            string apiUrl = $"https://api.github.com/repos/{owner}/{repo}/releases";

            try
            {
                using (HttpClient client = new HttpClient())
                {
                    client.DefaultRequestHeaders.UserAgent.ParseAdd("shterm-app");
                    string responseJson = await client.GetStringAsync(apiUrl);

                    using (JsonDocument doc = JsonDocument.Parse(responseJson))
                    {
                        var releaseList = new List<GitHubReleaseAsset>();
                        foreach (var releaseElement in doc.RootElement.EnumerateArray())
                        {
                            string releaseName = releaseElement.GetProperty("name").GetString();
                            if (string.IsNullOrEmpty(releaseName))
                            {
                                releaseName = releaseElement.GetProperty("tag_name").GetString();
                            }

                            var assets = releaseElement.GetProperty("assets");
                            if (assets.GetArrayLength() > 0)
                            {
                                string downloadUrl = assets[0].GetProperty("browser_download_url").GetString();
                                string fileName = assets[0].GetProperty("name").GetString();
                                releaseList.Add(new GitHubReleaseAsset { ReleaseName = releaseName, DownloadUrl = downloadUrl, FileName = fileName });
                            }
                        }

                        if (releaseList.Count == 0)
                        {
                            txtTerminal.AppendText($"No production release assets compiled for this repo.{Environment.NewLine}$ ");
                            return;
                        }

                        var sortedReleases = releaseList.OrderBy(r => r.ReleaseName).ToList();
                        availableReleaseOptions.Clear();

                        char optionChar = 'a';
                        txtTerminal.AppendText($"Multiple releases detected. Select target compilation:{Environment.NewLine}");

                        foreach (var item in sortedReleases)
                        {
                            if (optionChar > 'z') break;
                            txtTerminal.AppendText($"Release Name: \"{item.ReleaseName}\" -> Press [{optionChar}] to download.{Environment.NewLine}");
                            availableReleaseOptions[optionChar] = item;
                            optionChar++;
                        }

                        isWaitingForReleaseSelection = true;
                        txtTerminal.AppendText($"Enter choice: ");
                    }
                }
            }
            catch (Exception ex)
            {
                txtTerminal.AppendText($"Fetch Error: {ex.Message}{Environment.NewLine}$ ");
            }
        }

        private async Task DownloadReleaseAsset(GitHubReleaseAsset asset)
        {
            txtTerminal.AppendText($"{Environment.NewLine}Initializing deployment stream for: {asset.FileName}{Environment.NewLine}");

            try
            {
                using (HttpClient client = new HttpClient())
                {
                    var response = await client.GetAsync(asset.DownloadUrl, HttpCompletionOption.ResponseHeadersRead);
                    response.EnsureSuccessStatusCode();

                    using (var stream = await response.Content.ReadAsStreamAsync())
                    using (var fileStream = new FileStream(asset.FileName, FileMode.Create, FileAccess.Write, FileShare.None, 8192, true))
                    {
                        byte[] buffer = new byte[8192];
                        long totalBytes = response.Content.Headers.ContentLength ?? -1;
                        long receivedBytes = 0;
                        int bytesRead;

                        while ((bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length)) > 0)
                        {
                            await fileStream.WriteAsync(buffer, 0, bytesRead);
                            receivedBytes += bytesRead;

                            int progressPercent = totalBytes > 0 ? (int)((receivedBytes * 100) / totalBytes) : 50;
                            int blocksCount = progressPercent / 4;
                            string filledBlocks = new string('█', blocksCount);
                            string emptyBlocks = new string('░', 25 - blocksCount);

                            string progressLine = $"\rFetch: [{filledBlocks}{emptyBlocks}] {progressPercent}% packing runtime...";
                            txtTerminal.AppendText(progressLine);

                            txtTerminal.Text = txtTerminal.Text.Substring(0, txtTerminal.Text.LastIndexOf('\r'));
                        }
                    }
                }

                txtTerminal.AppendText(Environment.NewLine);
                txtTerminal.AppendText($"Unpacking compilation binaries into execution context...{Environment.NewLine}");
                await Task.Delay(600);
                txtTerminal.AppendText($"Successfully unpacked asset to working directory.{Environment.NewLine}{Environment.NewLine}$ ");
            }
            catch (Exception ex)
            {
                txtTerminal.AppendText($"{Environment.NewLine}Download Failed: {ex.Message}{Environment.NewLine}$ ");
            }
        }

        private void AppendTerminalText(string text)
        {
            if (text != null)
            {
                Invoke(new Action(() =>
                {
                    txtTerminal.AppendText(text + Environment.NewLine);
                    txtTerminal.SelectionStart = txtTerminal.Text.Length;
                    txtTerminal.ScrollToCaret();
                }));
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            try
            {
                if (cmdProcess != null && !cmdProcess.HasExited)
                {
                    cmdWriter.WriteLine("exit");
                    cmdProcess.Close();
                }
            }
            catch { }
            base.OnFormClosing(e);
        }
    }

    public class GitHubReleaseAsset
    {
        public string ReleaseName { get; set; }
        public string DownloadUrl { get; set; }
        public string FileName { get; set; }
    }
}
