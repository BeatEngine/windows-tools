// See https://aka.ms/new-console-template for more information
using System.Text.RegularExpressions;


class MKDocumentIndexer
{


    static string buildCategoryString(in List<string> fileMatchCategory)
    {
        string c = "";
        bool first = true;
        foreach(string cat in fileMatchCategory)
        {
            if(!first)
            {
                c += "|";
            }
            else
            {
                first = false;
            }
            c += cat;
        }
        return c;
    }

    static void saveResult(string pathToFile, in StreamWriter outFile, in List<string> fileMatchCategory)
    {
        if(outFile != null)
        {
            //File path;File changed;File category
            DateTime? modified = null;
            try
            {
                modified = File.GetLastWriteTime(pathToFile);
            } catch(Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
            if(modified == null)
            {
                modified = DateTime.MinValue;
            }
            outFile.WriteLine(pathToFile+"; "+modified+"; "+buildCategoryString(fileMatchCategory));
        }
    }

    static void writeCSVhead(StreamWriter outFile)
    {
        string head = "File path;File changed;File category";
        outFile.WriteLine(head);
    }

    static bool applyContains(in string fileBasename, in List<string> searchContains, bool all, ref List<string> fileMatchCategory)
    {
        foreach(string contains in searchContains)
        {
            if(all)
            {
                if(!fileBasename.Contains(contains))
                {
                    return false;
                }
            }
            else
            {
                if(fileBasename.Contains(contains))
                {
                    fileMatchCategory.Add(contains);
                    return true;
                }
            }
        }
        
        if(all && searchContains.Count() > 0)
        {
            fileMatchCategory.AddRange(searchContains);
            return true;
        }
        return false;
    }

    static bool applyEnds(in string fileBasename, in List<string> searchEnds, bool all, ref List<string> fileMatchCategory)
    {
        foreach(string ends in searchEnds)
        {
            if(all)
            {
                if(!fileBasename.EndsWith(ends))
                {
                    return false;
                }
            }
            else
            {
                if(fileBasename.EndsWith(ends))
                {
                    fileMatchCategory.Add(ends);
                    return true;
                }
            }
        }

        if(all && searchEnds.Count() > 0)
        {
            fileMatchCategory.AddRange(searchEnds);
            return true;
        }
        return false;
    }

    static string baseName(string path)
    {
        int i = path.LastIndexOf("\\");
        if(i >= 0)
        {
            return path.Substring(i+1);
        }
        return path;
    }

    static void applyFilters(string pathToFile, in StreamWriter outFile, in List<string> searchContains, in List<string> searchEnds, in bool andFilter)
    {
        List<string> fileMatchCategory = new List<string>();
        string bana = baseName(pathToFile);
        if(andFilter)
        {
            // Add every category (all) by every internal match 
            if(applyEnds(bana,searchEnds, true, ref fileMatchCategory) && applyContains(bana,searchContains, true, ref fileMatchCategory))
            {
                // IF ALL ENDS AND CONTAINS matches
                // Found file add to index
                saveResult(pathToFile, outFile, fileMatchCategory);
            }
        }
        else //OR
        {
            // Add every category by every internal match 
            if(applyEnds(bana,searchEnds, false, ref fileMatchCategory) || applyContains(bana,searchContains, false, ref fileMatchCategory))
            {
                // IF any ENDS or CONTAINS matches
                // Found file add to index
                saveResult(pathToFile, outFile, fileMatchCategory);
            }
        }
    }


    static string unquote(string str)
    {
        if(str.StartsWith("\"") && str.EndsWith("\""))
        {
            return str.Substring(1, str.Length-2);
        }
        return str;
    }

    private static void IterateSerachPath(in string pathDestination, in string searchPath, in List<string> searchContains, in List<string> searchEnds, in bool andFilter, in StreamWriter outFile)
    {
        try
        {
            // Iterate over all files in the current directory
            foreach (string file in Directory.EnumerateFiles(searchPath))
            {
                applyFilters(file, outFile, searchContains, searchEnds, andFilter);
            }

            // Recursively iterate over all subdirectories
            foreach (string subdirectory in Directory.EnumerateDirectories(searchPath))
            {
                IterateSerachPath(pathDestination, subdirectory, searchContains, searchEnds, andFilter, outFile); // Recursively call the method
            }
        }
        catch (UnauthorizedAccessException)
        {
            // Handle access errors, for example, permissions issues
            Console.WriteLine($"Access denied to directory: {searchPath}");
        }
        catch (Exception ex)
        {
            // Handle other exceptions
            Console.WriteLine($"Error accessing directory {searchPath}: {ex.Message}");
        }
    }

    static void IterateFiles(in string pathDestination, in List<string> searchPaths, in List<string> searchContains, in List<string> searchEnds, in bool append, in bool andFilter)
    {
        using(StreamWriter outputFile = new StreamWriter(Path.Combine(pathDestination, "document-index.csv"), append, System.Text.Encoding.UTF8))
        {
            if(!append)
            {
                writeCSVhead(outputFile);
            }
            foreach(string path in searchPaths)
            {
                IterateSerachPath(pathDestination, path, searchContains, searchEnds, andFilter, outputFile);
            }
            outputFile.Flush();
        }
    }

    static void Main(string[] args)
    {
        
        if(args.Length < 2)
        {
            //HELP

            Console.WriteLine("How to use:");
            String filename = System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName;
            filename = filename.Substring(filename.LastIndexOf('\\')+1);
            Console.WriteLine("!!! You can use as many search paths you want, but at least one !!!");
            Console.WriteLine("!!! The file document-index.csv will be created in the \"Destination-Path\" !!!");
            Console.WriteLine(filename + " -search \"Search-Path1\" -search \"Search-Path2\" -search \"Search-Path3\" -o \"Destination-Path\" [SEARCH-OPTIONS]");
            Console.WriteLine("[SEARCH-OPTIONS] <---> filter which files you want to index !! each combinations with logical OR !!");
            Console.WriteLine($"-append          <---> !! document-index.csv will not be overwritten, but appended for usage of '{filename}' in scripts!! ");
            Console.WriteLine("-and             <---> !! each combinations with logical AND !! for example -and -fe \".doc\" -fc \"test\" --> all files whith 'test' in name of type 'doc'");
            Console.WriteLine("-fe \".pdf\"       <---> file ends with \"pdf\" --> would index all pdfs");
            Console.WriteLine("-fc \".pdf\"       <---> file contains \"SCO\" --> would index all files with \"SCO\" in base-name like path\\base-name.xyz");
            Console.WriteLine("\nExample: -fe \"z1.pdf\" -fe \"z2.pdf\" -fe \"y1.cfg\" -fe \"y2.cfg\" -fc \"ABC\"");
            Console.WriteLine($"\nFull-Example:\n{filename} -search \"C:\\Customers\\\" -o \".\\\" -fe \"z.pdf\" -fe \"y.pdf\" -fe \"z.cfg\" -fe \"y.cfg\" -fc \"SCO\"");
            Console.ReadKey();
        }
        else
        {
            List<string> searchPaths = new List<string>();
            List<string> searchContains = new List<string>();
            List<string> searchEnds = new List<string>();
            string pathDestination = ".\\";
            bool append = false;
            bool andFilter = false;

            // Get Arguments

            for(int i = 0; i < args.Length; i++)
            {
                
                if("-search".Equals(args[i]) && i+1 < args.Length)
                {
                    searchPaths.Add(unquote(args[i+1]));
                }
                else if("-o".Equals(args[i]) && i+1 < args.Length)
                {
                    pathDestination = unquote(args[i+1]);
                }
                else if("-fe".Equals(args[i]) && i+1 < args.Length)
                {
                    searchEnds.Add(unquote(args[i+1]));
                }
                else if("-fc".Equals(args[i]) && i+1 < args.Length)
                {
                    searchContains.Add(unquote(args[i+1]));
                }
                else if("-append".Equals(args[i]))
                {
                    append = true;
                }
                else if("-and".Equals(args[i]))
                {
                    andFilter = true;
                }
                else if("-search".Equals(args[i]) || "-o".Equals(args[i]) || "-fe".Equals(args[i]) || "-fc".Equals(args[i]))
                {
                    Console.Error.WriteLine("Wrong arguments: " + buildCategoryString(args.ToList()).Replace('|', ','));
                    return;
                }

            }


            // RUN
            IterateFiles(pathDestination, searchPaths, searchContains, searchEnds, append, andFilter);
        }

    }
}

