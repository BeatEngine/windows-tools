using System;
using System.Diagnostics;
using System.Speech.Synthesis;
using NAudio.Wave;


  class Program
  {

    static void downloadAudio(string filename, string url)
    {
#pragma warning disable SYSLIB0014 // Type or member is obsolete
        using (var webClient = new System.Net.WebClient())
        {
            try
            {
                File.WriteAllBytes(filename, webClient.DownloadData(url));
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
            }
        }
#pragma warning restore SYSLIB0014 // Type or member is obsolete
    }
    

     static void appendAudioFilesToWave(string outputFile, List<string> inputFiles, int format = 0)
    {
        if(format == 0)
        {
            using WaveFileReader r = new WaveFileReader(inputFiles[0]);
            using var writer = new WaveFileWriter(outputFile, r.WaveFormat);

            foreach (var file in inputFiles)
            {
                using var reader = new WaveFileReader(file);

                if (!reader.WaveFormat.Equals(writer.WaveFormat))
                {
                    throw new InvalidOperationException("Alle WAV-Dateien müssen dasselbe Format haben!");
                }

                reader.CopyTo(writer);
            }
        }
        else
        {
            using Mp3FileReader r = new Mp3FileReader(inputFiles[0]);
            Mp3WaveFormat wf = r.Mp3WaveFormat;
            using var writer = new WaveFileWriter(outputFile, new WaveFormat(wf.SampleRate, wf.Channels));
            foreach (var file in inputFiles)
            {
                using var reader = new Mp3FileReader(file);
                reader.CopyTo(writer);
            }
        }
    }


    static void exportGoogle(string destination, string csvDictionary, string languageA, string languageB)
    {
        string appData = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        List<string> tmpFiles = new List<string>();
        string tmpDir = Path.Combine(appData, "tmp-audio-vocabtts");
        int fileNumber = 1;
        Directory.CreateDirectory(tmpDir);
        Console.WriteLine("");

        // Request google translate audios
        string startUrl = "https://translate.google.com/translate_tts?ie=UTF-8&client=gtx";

        ThreadPool.SetMaxThreads(3, 3);

        List<Task> tasks = new List<Task>();

        const Int32 BufferSize = 128;
        using (var fileStream = File.OpenRead(csvDictionary)) {
        using (var streamReader = new StreamReader(fileStream, System.Text.Encoding.UTF8, true, BufferSize)) {
            String line;
   
            while ((line = streamReader.ReadLine()) != null)
            {
                int coloneIndex = line.IndexOf(';');
                if(coloneIndex > 0)
                {
                    string spanishWord = line.Substring(0, coloneIndex);
                    string germanWord = line.Substring(coloneIndex+1);
                    Console.Write("\r" + spanishWord + " - " + germanWord + "                                                          ");
                    // Speak Spanish
                    string tf = Path.Combine(tmpDir, $"tmp{fileNumber}.mp3");
                    string url = $"{startUrl}&q={Uri.EscapeDataString(spanishWord)}&tl={languageA}";

                    while(tasks.Count() > 2)
                    {
                        for(int t = tasks.Count()-1; t >= 0; t--)
                        {
                            if(tasks[t].IsCompleted)
                            {
                                tasks.RemoveAt(t);
                            }
                        }
                    }

                    tasks.Add(Task.Run(()=> {
                        downloadAudio(tf, url);
                    }));
                    
                    tmpFiles.Add(tf);
                    fileNumber++;
                    // Speak German
                    string tf2 = Path.Combine(tmpDir, $"tmp{fileNumber}.mp3");
                    string url2 = $"{startUrl}&q={Uri.EscapeDataString(germanWord)}&tl={languageB}";
                    tasks.Add(Task.Run(()=> {
                        downloadAudio(tf2, url2);
                    }));
                    tmpFiles.Add(tf2);
                    fileNumber++;
                }
            }

            while(tasks.Count() > 2)
            {
                for(int t = tasks.Count()-1; t >= 0; t--)
                {
                    if(tasks[t].IsCompleted)
                    {
                        tasks.RemoveAt(t);
                    }
                }
            }
    

            
        }
        }
        


        Console.WriteLine("\nBuild output " + destination);
        
        appendAudioFilesToWave(destination, tmpFiles, 1);

        // Clean tmp files
        Console.WriteLine("Cleanup temporary files...");
        tmpFiles.ForEach(x => File.Delete(x));
        Directory.Delete(tmpDir);
    }
    

    static void exportAudios(string destination, string csvDictionary, string languageA, string languageB)
    {
        string appData = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        List<string> tmpFiles = new List<string>();
        string tmpDir = Path.Combine(appData, "tmp-audio-vocabtts");
        using(SpeechSynthesizer synth = new SpeechSynthesizer())
        {
        Console.WriteLine("Installed TTS-Languages:");
        foreach (var voice in synth.GetInstalledVoices())
        {
            var info = voice.VoiceInfo;
            Console.WriteLine($"Name: {info.Name}, Sprache: {info.Culture}");
        }

        int fileNumber = 1;
        
        
        Directory.CreateDirectory(tmpDir);

        
        Console.WriteLine("");
        const Int32 BufferSize = 128;
        using (var fileStream = File.OpenRead(csvDictionary)) {
        using (var streamReader = new StreamReader(fileStream, System.Text.Encoding.UTF8, true, BufferSize)) {
            String line;
            while ((line = streamReader.ReadLine()) != null)
            {
                int coloneIndex = line.IndexOf(';');
                if(coloneIndex > 0)
                {
                    
                    string spanishWord = line.Substring(0, coloneIndex);
                    string germanWord = line.Substring(coloneIndex+1);
                    Console.Write("\r" + spanishWord + " - " + germanWord + "                                                          ");

                    // Speak Spanish
                    synth.SelectVoiceByHints(VoiceGender.Male, VoiceAge.Adult, 0, new System.Globalization.CultureInfo(languageA));
                    string tf = Path.Combine(tmpDir, $"tmp{fileNumber}.wav");
                    synth.SetOutputToWaveFile(tf);
                    synth.Speak(spanishWord);
                    synth.SetOutputToNull();
                    tmpFiles.Add(tf);
                    fileNumber++;
                    // Speak German
                    synth.SelectVoiceByHints(VoiceGender.Male, VoiceAge.Adult, 0, new System.Globalization.CultureInfo(languageB));
                    tf = Path.Combine(tmpDir, $"tmp{fileNumber}.wav");
                    synth.SetOutputToWaveFile(tf);
                    synth.Speak(germanWord);
                    synth.SetOutputToNull();
                    tmpFiles.Add(tf);
                    fileNumber++;
                }
            }
        }
        }
        }

        Console.WriteLine("\nBuild output " + destination);
        
        appendAudioFilesToWave(destination, tmpFiles);

        // Clean tmp files
        Console.WriteLine("Cleanup temporary files...");
        tmpFiles.ForEach(x => File.Delete(x));
        Directory.Delete(tmpDir);
    }

    static String spaces(int l)
    {
        String x = "";
        for(int c = 0; c < l; c++)
        {
            x += " ";
        }
        return x;
    }

    static void Main(string[] args)
    {
        if(args.Length < 4)
        {
            //HELP
            Console.WriteLine("How to use:");
            String filename = System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName;
            filename = filename.Substring(filename.LastIndexOf('\\')+1);
            Console.WriteLine(filename + " \"csv-file-with-2-columns-translation.csv\" \"output-file.wav\" \"LANGUAGE-A\" \"LANGUAGE-B\" [-google]");
            Console.WriteLine(spaces(filename.Count()) + "                                              This option uses google translator voice [-google]");
            Console.WriteLine("Example: !!!You need to install the language in Windows!!!");
            Console.WriteLine(filename + " \"spanish-german-vocab.csv\" \"span-germ.wav\" \"es-ES\" \"de-DE\"");
            SpeechSynthesizer synth = new SpeechSynthesizer();
            Console.WriteLine("Installed TTS-Languages:");
            foreach (var voice in synth.GetInstalledVoices())
            {
                var info = voice.VoiceInfo;
                Console.WriteLine($"Name: {info.Name}, Sprache: {info.Culture}");
            }
            Console.ReadKey();
            return;
        }

        string csv = args[0];
        string wav = args[1];
        string languageA = args[2];
        string languageB = args[3];
        bool googleTranslator = false;
        if(args.Length > 4)
        {
            if("-google".Equals(args[4]))
            {
                googleTranslator = true;
            }
        }

      // Initialize a new instance of the SpeechSynthesizer.

      try {
      if(googleTranslator)
      {
        exportGoogle(wav, csv, languageA.Substring(0, languageA.IndexOf('-')), languageB.Substring(0, languageB.IndexOf('-')));
      }
      else
      {
        exportAudios(wav, csv, languageA, languageB);
      }
      }
      catch(Exception e)
      {
        Console.WriteLine(e.Message);
      }

      

      Console.WriteLine();
      Console.WriteLine("Use the following command with ffmpeg to convert:");

      
      
      string cmd = $"ffmpeg -i \"{Path.GetFullPath(wav)}\" -vn -ar 44100 -ac 1 -b:a 160k \"{Path.GetFullPath(wav.Replace(".wav", ".mp3"))}\"";
      Console.WriteLine(cmd);
      Console.WriteLine($"Do you want to convert to mp3 ({wav.Replace(".wav", ".mp3")}) and delete the {wav} ? (y/n):");

      if(Console.ReadLine().StartsWith("y"))
      {
        Process p = Process.Start(cmd);
        p.WaitForExit(10*60*60*1000);
        File.Delete(wav);
      }

    }

}




   



