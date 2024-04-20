using Community.VisualStudio.Toolkit;
using Microsoft;
using Microsoft.VisualStudio.Extensibility;
using Microsoft.VisualStudio.Extensibility.Commands;
using Microsoft.VisualStudio.Extensibility.Documents;
using Microsoft.VisualStudio.ProjectSystem.Query;
using Microsoft.VisualStudio.RpcContracts.Documents;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Threading;
using Microsoft.VisualStudio.VCProjectEngine;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Net;

namespace WinUI3XamlPreviewVS2022
{
    internal struct PreviewInfo
    {
        public string AppPath;
        public string FilePath;
    }

    [VisualStudioContribution]
    internal class ToggleLivePreview : Command, IToggleCommand, IDocumentEventsListener
    {
        private readonly TraceSource _logger;
        private OutputWindow? _outWindow;
        private IDisposable? _documentSub;
        private PreviewInfo? _lastOpenedInfo;
        private CancellationTokenSource? _lastOpenedAppWaitSource;

        public ToggleLivePreview(TraceSource traceSource)
        {
            _logger = Requires.NotNull(traceSource, nameof(traceSource));
        }

        /// <inheritdoc />
        public override CommandConfiguration CommandConfiguration => new("%WinUI3XamlPreviewVS2022.ToggleLivePreview.DisplayName%")
        {
            // Use this object initializer to set optional parameters for the command. The required parameter,
            // displayName, is set above. DisplayName is localized and references an entry in .vsextension\string-resources.json.
            Icon = new(ImageMoniker.KnownValues.MarkupXML, IconSettings.IconAndText),
            Placements = [CommandPlacement.KnownPlacements.ExtensionsMenu],
        };

        public bool IsChecked => _documentSub != null;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            TurnOffPreview();
        }

        /// <inheritdoc />
        public override async Task InitializeAsync(CancellationToken cancellationToken)
        {
            // Use InitializeAsync for any one-time setup or initialization.
            await base.InitializeAsync(cancellationToken);
            _outWindow = await Extensibility.Views().Output.GetChannelAsync("WinUI3XamlPreview", "OutputName", cancellationToken);
        }

        /// <inheritdoc />
        public override async Task ExecuteCommandAsync(IClientContext context, CancellationToken cancellationToken)
        {
            if (IsChecked)
            {
                TurnOffPreview();
            }
            else
            {
                _documentSub = await Extensibility.Documents().SubscribeAsync(this, null, cancellationToken);
                await TryOpenActiveDocumentAsync(cancellationToken);
                OnPropertyChanged(new PropertyChangedEventArgs(nameof(IsChecked)));
            }
        }
        private async Task TryOpenActiveDocumentAsync(CancellationToken cancellationToken)
        {
            var doc = await VS.Documents.GetActiveDocumentViewAsync();
            if (doc == null)
            {
                return;
            }
            await OpenAsync(doc.FilePath ?? "", cancellationToken);
        }

        private void TurnOffPreview()
        {
            _documentSub?.Dispose();
            _documentSub = null;
            TryCloseLastOpenedApp();
            _lastOpenedInfo = null;
            _lastOpenedAppWaitSource?.Cancel();
            OnPropertyChanged(new PropertyChangedEventArgs(nameof(IsChecked)));
        }

        private void TryCloseLastOpenedApp()
        {
            if (_lastOpenedInfo?.AppPath is not string previewAppPath)
            {
                return;
            }
            Process.Start(previewAppPath, $"----ms-protocol:winui3xp://close");
        }

        private async Task OpenAsync(string path, CancellationToken cancellationToken)
        {
            try
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                if (await VS.Solutions.GetActiveItemAsync() is not PhysicalFile activeFile)
                {
                    return;
                }
                var docProject = activeFile.ContainingProject;
                if (docProject == null)
                {
                    return;
                }
                var isExeProjectCpp = (await docProject.GetAttributeAsync("ConfigurationType")) == "Application";
                var isExeProjectCs = (await docProject.GetAttributeAsync("OutputType")) == "WinExe";
                var customLaunchProjectName = await docProject.GetAttributeAsync("WinUI3XPLaunchProject");
                var isExeProjectCustom = customLaunchProjectName != null;
                var isExeProject = isExeProjectCpp || isExeProjectCs || isExeProjectCustom;
                string appPath;
                string host;
                string queries;
                if (isExeProject)
                {
                    if (isExeProjectCustom)
                    {
                        docProject = await VS.Solutions.FindProjectsAsync(customLaunchProjectName ?? "") ?? throw new InvalidOperationException($"Project {customLaunchProjectName} doesn't exist");
                    }
                    (appPath, host, queries) = await OpenExeProjectAsync(docProject, cancellationToken);
                }
                else
                {
                    // Assume it's DLL project. WinRT doesn't support static library anyways.
                    (appPath, host, queries) = await OpenDllProjectAsync(docProject);
                }
                var urlencodedPath = WebUtility.UrlEncode(path);
                var urlencodedQueries = WebUtility.UrlEncode(queries);
                var isDifferentApp = _lastOpenedInfo?.AppPath != appPath;
                if (isDifferentApp)
                {
                    TryCloseLastOpenedApp();
                }
                if (_documentSub == null)
                {
                    return;
                }
                var arguments = $"----ms-protocol:winui3xp://{host}?filePath={urlencodedPath}&{queries}";
                var process = Process.Start(appPath, arguments);
                if (process == null)
                {
                    throw new InvalidOperationException("Could not start the app process");
                }
                var shouldWaitForExit = isDifferentApp;
                _lastOpenedInfo = new PreviewInfo
                {
                    AppPath = appPath,
                    FilePath = path,
                };
                // For the same app, the process would exit immediately due to redirect so we don't wait for them
                if (shouldWaitForExit)
                {
                    _lastOpenedAppWaitSource?.Cancel();
                    _lastOpenedAppWaitSource = new CancellationTokenSource();
                    _ = TryWaitForAppProcessAsync(process);
                }
            }
            catch (Exception ex)
            {
                _outWindow?.Writer.WriteLineAsync(ex.ToString());
                await VS.StatusBar.ShowMessageAsync($"Failed to start preview: {ex}");
            }
        }

        private async Task<string> GetOutputFullPathAsync(Project docProject, string outDir, string fileName)
        {
            string fullPath = Path.Combine(outDir ?? "", fileName);
            // Cs project outdir is relative...Use uri to detect. Relative path somehow throw in ctor
            try
            {
                var uri = new Uri(fullPath);
            }
            catch (Exception)
            {
                var projectDir = await docProject.GetAttributeAsync("ProjectDir");
                fullPath = Path.Combine(projectDir, fullPath);
            }
            return fullPath;
        }
        class PackageFormat { };
        class SinglePackageProject : PackageFormat { }
        class Wap(string targetName) : PackageFormat { public string TargetName => targetName; }

        private async Task<PackageFormat?> CheckPackageFormatAsync(Project docProject, CancellationToken token)
        {
            var isSinglePackageProject = (await docProject.GetAttributeAsync("AppxPackage")) == "true";
            if (isSinglePackageProject)
            {
                return new SinglePackageProject();
            }
            const string entryPointPropName = "EntryPointProjectUniqueName";
            var packagingProjects = await VS.Solutions.GetAllProjectsAsync();
            var docProjectPath = docProject.FullPath;
            foreach (var packagingProject in packagingProjects)
            {
                var entryPoint = await packagingProject.GetAttributeAsync(entryPointPropName);
                if (!string.IsNullOrEmpty(entryPoint))
                {
                    try
                    {
                        new Uri(entryPoint);
                    }
                    catch
                    {
                        // entryPoint is relative.
                        var packagingProjectDir = await packagingProject.GetAttributeAsync("ProjectDir");
                        entryPoint = Path.GetFullPath(Path.Combine(packagingProjectDir, entryPoint));
                    }
                    if (entryPoint == docProjectPath)
                    {
                        return new Wap(await packagingProject.GetAttributeAsync("TargetName") ?? throw new InvalidOperationException("Project doesn't have TargetName"));
                    }
                }
            }
            return null;
        }

        private async Task<(string appPath, string host, string quries)> OpenExeProjectAsync(Project docProject, CancellationToken token)
        {
            var targetName = await docProject.GetAttributeAsync("TargetName");
            var packageFormat = await CheckPackageFormatAsync(docProject, token);
            string previewAppPath;
            if (packageFormat is SinglePackageProject)
            {
                previewAppPath = $"{targetName}-WinUI3XP";
            }
            else if (packageFormat is Wap wap)
            {
                previewAppPath = $"{wap.TargetName}-WinUI3XP";
            }
            else
            {
                var outDir = await docProject.GetAttributeAsync("OutDir");
                previewAppPath = await GetOutputFullPathAsync(docProject, outDir ?? "", $"{targetName}.exe" ?? "");
            }
            return (previewAppPath, "show", "");
        }

        private async Task<bool> IsDllProjectAsync(Project project)
        {
            var isDllProjectCpp = (await project.GetAttributeAsync("ConfigurationType")) == "DynamicLibrary";
            var isDllProjectCs = (await project.GetAttributeAsync("OutputType")) != "WinExe";
            return isDllProjectCpp || isDllProjectCs;
        }

        private async Task<List<string>> GetDllPathsAsync(Project project, CancellationToken token)
        {
            var dllPaths = new List<string>();
            var vsProject = (await this.Extensibility.Workspaces()
                .QueryProjectsAsync(q => q.With(p => p.ProjectReferences.With(r => r.Name))
                    .Where(p => p.Path == project.FullPath), token)).First();
            foreach (var reference in vsProject.ProjectReferences)
            {
                var dependentProject = await VS.Solutions.FindProjectsAsync(reference.Name);
                if (dependentProject == null)
                {
                    continue;
                }
                var isDllProject = await IsDllProjectAsync(dependentProject);
                if (!isDllProject)
                {
                    // Not a dll project but is referenced, assume they are not relevant (static lib/exe) for
                    // the purpose of loadLibrary
                    continue;
                }
                // Order is important: we must get dependent dll first, otherwise loadLibrary would fail
                dllPaths.AddRange(await GetDllPathsAsync(dependentProject, token));
            }
            var targetName = await project.GetAttributeAsync("TargetName");
            var outDir = await project.GetAttributeAsync("OutDir");
            var dllPath = await GetOutputFullPathAsync(project, outDir ?? "", $"{targetName}.dll");
            dllPaths.Add(dllPath);
            return dllPaths;
        }

        private async Task<(string appPath, string host, string quries)> OpenDllProjectAsync(Project docProject)
        {
            var source = new CancellationTokenSource();
            var projectName = await docProject.GetAttributeAsync("ProjectName");
            var outDir = await docProject.GetAttributeAsync("OutDir");
            var appPath = await GetOutputFullPathAsync(docProject, $"{outDir}..\\{projectName}_Preview", "WinUI3XamlPreview_DllLoader.exe");
            if (_lastOpenedInfo?.AppPath == appPath)
            {
                return (appPath, "show", "");
            }
            var dllPaths = await GetDllPathsAsync(docProject, source.Token);
            return (appPath, "loadLibrary", string.Join("&", dllPaths.Select(x => $"dllPath={WebUtility.UrlEncode(x)}")));
        }

        private async Task TryWaitForAppProcessAsync(Process process)
        {
            var source = _lastOpenedAppWaitSource;
            var lastOpendAppPath = _lastOpenedInfo?.AppPath;
            if (source == null)
            {
                throw new InvalidOperationException("Waiting process exist without a token source");
            }
            try
            {
                await process.WaitForExitAsync(source.Token);
                if (_lastOpenedInfo?.AppPath == lastOpendAppPath && _documentSub != null)
                {
                    TurnOffPreview();
                }
            }
            catch (TaskCanceledException)
            {
                // No-op
            }
            catch (Exception ex)
            {
                _outWindow?.Writer.WriteLineAsync(ex.ToString());
            }
        }

        private async Task TryOpenAsync(Uri uri, bool skipSameFile, CancellationToken token)
        {
            try
            {
                var doc = await VS.Documents.GetActiveDocumentViewAsync();
                // Fun: Doc and overall project use \, but for some reason moniker use /
                var openPath = uri.AbsolutePath.Replace("/", "\\");
                if (doc?.FilePath != openPath)
                {
                    return;
                }
                if (_lastOpenedInfo?.FilePath == openPath && skipSameFile)
                {
                    return;
                }
                await OpenAsync(openPath, token);
            }
            catch (Exception ex)
            {
                _outWindow?.Writer.WriteLineAsync(ex.ToString());
            }
        }

        public Task OpenedAsync(DocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }

        public Task ClosedAsync(DocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }

        public Task SavingAsync(DocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }

        public Task SavedAsync(DocumentEventArgs e, CancellationToken token)
        {
            if (_documentSub == null)
            {
                return Task.CompletedTask;
            }
            // For some reason calling some async method here would cause apparent deadlock
            // Workaround: Return immediately and start work in worker thread
            _ = Task.Run(async () =>
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                if (_documentSub == null)
                {
                    return;
                }
                var source = new CancellationTokenSource();
                await TryOpenAsync(e.Moniker, false, source.Token);
            });
            return Task.CompletedTask;
        }

        public Task RenamedAsync(RenamedDocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }

        public async Task ShownAsync(DocumentEventArgs e, CancellationToken token)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            if (_documentSub == null)
            {
                return;
            }
            await TryOpenAsync(e.Moniker, true, token);
        }

        public Task HiddenAsync(DocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }
    }
}
