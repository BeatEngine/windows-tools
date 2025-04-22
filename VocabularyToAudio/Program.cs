using System;
using System.Diagnostics;
using System.Speech.Synthesis;
using NAudio.Wave;


  class Program
  {

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
        using (var fileStream = File.OpenRead(csvDictionary))
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

        /*Console.Write("\n\nWant to open single files (y/n):");
        string files = Console.ReadLine();

        if("y".Equals(files))
        {
            Process.Start($"explorer.exe \"{tmpDir}\"");
            Console.Write("Continue delete files press enter:");
            Console.ReadLine();
        }*/

        Console.WriteLine("\nBuild output " + destination);
        
        appendWavFiles(destination, tmpFiles);

        // Clean tmp files
        Console.WriteLine("Cleanup temporary files...");
        tmpFiles.ForEach(x => File.Delete(x));
        Directory.Delete(tmpDir);
    }

    static void Main(string[] args)
    {
        if(args.Length < 4)
        {
            //HELP
            Console.WriteLine("How to use:");
            String filename = System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName;
            filename = filename.Substring(filename.LastIndexOf('\\')+1);
            Console.WriteLine(filename + " \"csv-file-with-2-columns-translation.csv\" \"output-file.wav\" \"LANGUAGE-A\" \"LANGUAGE-B\"");
            Console.WriteLine("Example: !!!You need to install the language in Windows!!!");
            Console.WriteLine(filename + " \"spanish-german-vocab.csv\" \"span-germ.wav\" \"es-ES\" \"de-DE\"");
            Console.ReadKey();
            return;
        }

        string csv = args[0];
        string wav = args[1];
        string languageA = args[2];
        string languageB = args[3];

      // Initialize a new instance of the SpeechSynthesizer.
      SpeechSynthesizer synth = new SpeechSynthesizer();
        Console.WriteLine("Installed TTS-Languages:");
        foreach (var voice in synth.GetInstalledVoices())
        {
            var info = voice.VoiceInfo;
            Console.WriteLine($"Name: {info.Name}, Sprache: {info.Culture}");
        }

      exportAudios(wav, csv, languageA, languageB);

      

      Console.WriteLine();
      //Console.WriteLine("Press any key to exit...");
      //Console.ReadKey();
    }




    static void appendWavFiles(string outputFile, List<string> inputFiles)
    {
        using var writer = new WaveFileWriter(outputFile, new WaveFileReader(inputFiles[0]).WaveFormat);

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


  }
