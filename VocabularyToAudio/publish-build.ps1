#dotnet publish -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true /p:PublishTrimmed=true /p:TrimMode=full
dotnet publish -c Release -r win-x64 --self-contained true /p:PublishTrimmed=false /p:EnableComHosting=true

