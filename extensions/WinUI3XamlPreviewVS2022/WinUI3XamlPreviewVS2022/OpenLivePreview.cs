using Community.VisualStudio.Toolkit;
using Microsoft;
using Microsoft.VisualStudio.Extensibility;
using Microsoft.VisualStudio.Extensibility.Commands;
using Microsoft.VisualStudio.Extensibility.Documents;
using Microsoft.VisualStudio.Extensibility.Shell;
using Microsoft.VisualStudio.ProjectSystem.Query;
using Microsoft.VisualStudio.RpcContracts.Documents;
using Microsoft.VisualStudio.Shell;
using Newtonsoft.Json.Linq;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Threading;
using System.Xml;

namespace WinUI3XamlPreviewVS2022
{
    internal static class AsyncEnumeratorExtension
    {
        public static async Task<T> FirstAsync<T>(this IAsyncEnumerator<T> iter)
        {
            if (! await iter.MoveNextAsync())
            {
                throw new InvalidOperationException();
            }
            return iter.Current;
        }
    }

    [VisualStudioContribution]
    internal class ToggleLivePreview : Command, IToggleCommand, IDocumentEventsListener
    {
        private readonly TraceSource _logger;
        private OutputWindow? _outWindow;
        private IDisposable? _documentSub;
        private string? _lastOpenedAppPath;

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
            ClosePreview();
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
                ClosePreview();
            }
            else
            {
                _documentSub = await Extensibility.Documents().SubscribeAsync(this, null, cancellationToken);
                await TryOpenActiveDocumentAsync(cancellationToken);
            }

            OnPropertyChanged(new PropertyChangedEventArgs(nameof(IsChecked)));
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

        private void ClosePreview()
        {
            _documentSub?.Dispose();
            _documentSub = null;
            TryCloseLastOpenedApp();
        }

        private void TryCloseLastOpenedApp()
        {
            if (_lastOpenedAppPath is not string previewAppPath)
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
                var targetName = await docProject.GetAttributeAsync("TargetName");
                var isPackaged = (await docProject.GetAttributeAsync("AppxPackage")) == "true";
                string previewAppPath;
                if (isPackaged)
                {
                    previewAppPath = $"{targetName}-WinUI3XP";
                }
                else
                {
                    var projectPath = docProject.FullPath;
                    var projectName = docProject.Name;
                    var outDir = await docProject.GetAttributeAsync("OutDir");
                    previewAppPath = Path.Combine(outDir ?? "", $"{targetName}.exe");
                    // Cs project outdir is relative...Use uri to detect. Relative path somehow throw in ctor
                    try
                    {
                        var uri = new Uri(previewAppPath);
                    }
                    catch (Exception ex)
                    {
                        var projectDir = await docProject.GetAttributeAsync("ProjectDir");
                        previewAppPath = Path.Combine(projectDir, previewAppPath);
                    }
                }
                var urlencodedPath = WebUtility.UrlEncode(path);
                // TODO: If opening a different project's file, close previous app
                Process.Start(previewAppPath, $"----ms-protocol:winui3xp://show?filePath=\"{urlencodedPath}\"");
                _lastOpenedAppPath = previewAppPath;
            }
            catch (Exception ex)
            {
                _outWindow?.Writer.WriteLineAsync(ex.ToString());
            }
        }

        private async Task TryOpenAsync(Uri uri, CancellationToken token)
        {
            try
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var doc = await VS.Documents.GetActiveDocumentViewAsync();
                // Fun: Doc and overall project use \, but for some reason moniker use /
                var openPath = uri.AbsolutePath.Replace("/", "\\");
                if (doc?.FilePath != openPath)
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
                var source = new CancellationTokenSource();
                await TryOpenAsync(e.Moniker, source.Token);
            });
            return Task.CompletedTask;
        }

        public Task RenamedAsync(RenamedDocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }

        public async Task ShownAsync(DocumentEventArgs e, CancellationToken token)
        {
            if (_documentSub == null)
            {
                return;
            }
            await TryOpenAsync(e.Moniker, token);
        }

        public Task HiddenAsync(DocumentEventArgs e, CancellationToken token)
        {
            return Task.CompletedTask;
        }
    }
}
