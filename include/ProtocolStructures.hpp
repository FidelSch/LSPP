#pragma once
#include <string>
#include "nlohmann/json.hpp"
#include <optional>


/**
 * Information about the server.
 *
 * @since 3.15.0
 */
struct ServerInfo
{
      /**
       * The name of the server as defined by the server.
       */
      std::string name;

      /**
       * The server's version as defined by the server.
       */
      std::string version;
};

/**
 * The capabilities the language server provides.
 */
struct ServerCapabilities
{
      /**
       * The position encoding the server picked from the encodings offered
       * by the client via the client capability `general.positionEncodings`.
       *
       * If the client didn't provide any position encodings the only valid
       * value that a server can return is 'utf-16'.
       *
       * If omitted it defaults to 'utf-16'.
       *
       * @since 3.17.0
       */
      struct PositionEncodingKind
      {
            static constexpr const char *encodings[] = {"utf-8", "utf-16"};
            enum encoding_index
            {
                  UTF8 = 0,
                  UTF16 = 1
            };
      };
      std::string positionEncoding;

      struct TextDocumentSyncOptions
      {
            /**
             * Open and close notifications are sent to the server. If omitted open
             * close notifications should not be sent.
             */
            bool openClose;

            /**
             * Change notifications are sent to the server. See
             * TextDocumentSyncKind.None, TextDocumentSyncKind.Full and
             * TextDocumentSyncKind.Incremental. If omitted it defaults to
             * TextDocumentSyncKind.None.
             */
            enum TextDocumentSyncKind
            {
                  None,
                  Full,
                  Incremental
            };
            TextDocumentSyncKind change;
      };

      /**
       * Defines how text documents are synced. Is either a detailed structure
       * defining each notification or for backwards compatibility the
       * TextDocumentSyncKind number. If omitted it defaults to
       * `TextDocumentSyncKind.None`.
       */
      TextDocumentSyncOptions::TextDocumentSyncKind textDocumentSync; 

      uint64_t advertisedCapabilities;

      enum
      {
            completionProvider               = 0x00000001,
            hoverProvider                    = 0x00000002,
            signatureHelpProvider            = 0x00000004,
            declarationProvider              = 0x00000008,
            definitionProvider               = 0x00000010,
            typeDefinitionProvider           = 0x00000020,
            implementationProvider           = 0x00000040,
            referencesProvider               = 0x00000080,
            documentHighlightProvider        = 0x00000100,
            documentSymbolProvider           = 0x00000200,
            codeActionProvider               = 0x00000400,
            codeLensProvider                 = 0x00000800,
            documentLinkProvider             = 0x00001000,
            colorProvider                    = 0x00002000,
            documentFormattingProvider       = 0x00004000,
            documentRangeFormattingProvider  = 0x00008000,
            documentOnTypeFormattingProvider = 0x00010000,
            renameProvider                   = 0x00020000,
            foldingRangeProvider             = 0x00040000,
            executeCommandProvider           = 0x00080000,
            selectionRangeProvider           = 0x00100000,
            linkedEditingRangeProvider       = 0x00200000,
            callHierarchyProvider            = 0x00400000,
            semanticTokensProvider           = 0x00800000,
            monikerProvider                  = 0x01000000,
            typeHierarchyProvider            = 0x02000000,
            inlineValueProvider              = 0x04000000,
            inlayHintProvider                = 0x08000000,
            diagnosticProvider               = 0x10000000,
            workspaceSymbolProvider          = 0x20000000
      };

      /**
       * Defines how notebook documents are synced.
       *
       * @since 3.17.0
       */
      //notebookDocumentSync ?: NotebookDocumentSyncOptions | NotebookDocumentSyncRegistrationOptions;

      /**
       * The server provides completion support.
       */
      // completionProvider ?: CompletionOptions;


      /**
       * The server provides hover support.
       */
      // hoverProvider ?: boolean | HoverOptions;

      /**
       * The server provides signature help support.
       */
      // signatureHelpProvider ?: SignatureHelpOptions;

      /**
       * The server provides go to declaration support.
       *
       * @since 3.14.0
       */
      // declarationProvider ?: boolean | DeclarationOptions | DeclarationRegistrationOptions;

      /**
       * The server provides goto definition support.
       */
      // bool definitionProvider;

      /**
       * The server provides goto type definition support.
       *
       * @since 3.6.0
       */
      // typeDefinitionProvider ?: boolean | TypeDefinitionOptions | TypeDefinitionRegistrationOptions;

      /**
       * The server provides goto implementation support.
       *
       * @since 3.6.0
       */
      // implementationProvider ?: boolean | ImplementationOptions | ImplementationRegistrationOptions;

      /**
       * The server provides find references support.
       */
      // referencesProvider ?: boolean | ReferenceOptions;

      /**
       * The server provides document highlight support.
       */
      // documentHighlightProvider ?: boolean | DocumentHighlightOptions;

      /**
       * The server provides document symbol support.
       */
      // documentSymbolProvider ?: boolean | DocumentSymbolOptions;

      /**
       * The server provides code actions. The `CodeActionOptions` return type is
       * only valid if the client signals code action literal support via the
       * property `textDocument.codeAction.codeActionLiteralSupport`.
       */
      // codeActionProvider ?: boolean | CodeActionOptions;

      /**
       * The server provides code lens.
       */
      // codeLensProvider ?: CodeLensOptions;

      /**
       * The server provides document link support.
       */
      // documentLinkProvider ?: DocumentLinkOptions;

      /**
       * The server provides color provider support.
       *
       * @since 3.6.0
       */
      // colorProvider ?: boolean | DocumentColorOptions | DocumentColorRegistrationOptions;

      /**
       * The server provides document formatting.
       */
      // documentFormattingProvider ?: boolean | DocumentFormattingOptions;

      /**
       * The server provides document range formatting.
       */
      // documentRangeFormattingProvider ?: boolean | DocumentRangeFormattingOptions;

      /**
       * The server provides document formatting on typing.
       */
      // documentOnTypeFormattingProvider ?: DocumentOnTypeFormattingOptions;

      /**
       * The server provides rename support. RenameOptions may only be
       * specified if the client states that it supports
       * `prepareSupport` in its initial `initialize` request.
       */
      // renameProvider ?: boolean | RenameOptions;

      /**
       * The server provides folding provider support.
       *
       * @since 3.10.0
       */
      // foldingRangeProvider ?: boolean | FoldingRangeOptions | FoldingRangeRegistrationOptions;

      /**
       * The server provides execute command support.
       */
      // executeCommandProvider ?: ExecuteCommandOptions;

      /**
       * The server provides selection range support.
       *
       * @since 3.15.0
       */
      // selectionRangeProvider ?: boolean | SelectionRangeOptions | SelectionRangeRegistrationOptions;

      /**
       * The server provides linked editing range support.
       *
       * @since 3.16.0
       */
      // linkedEditingRangeProvider ?: boolean | LinkedEditingRangeOptions | LinkedEditingRangeRegistrationOptions;

      /**
       * The server provides call hierarchy support.
       *
       * @since 3.16.0
       */
      // callHierarchyProvider ?: boolean | CallHierarchyOptions | CallHierarchyRegistrationOptions;

      /**
       * The server provides semantic tokens support.
       *
       * @since 3.16.0
       */
      // semanticTokensProvider ?: SemanticTokensOptions | SemanticTokensRegistrationOptions;

      /**
       * Whether server provides moniker support.
       *
       * @since 3.16.0
       */
      // monikerProvider ?: boolean | MonikerOptions | MonikerRegistrationOptions;

      /**
       * The server provides type hierarchy support.
       *
       * @since 3.17.0
       */
      // typeHierarchyProvider ?: boolean | TypeHierarchyOptions | TypeHierarchyRegistrationOptions;

      /**
       * The server provides inline values.
       *
       * @since 3.17.0
       */
      // inlineValueProvider ?: boolean | InlineValueOptions | InlineValueRegistrationOptions;

      /**
       * The server provides inlay hints.
       *
       * @since 3.17.0
       */
      // inlayHintProvider ?: boolean | InlayHintOptions | InlayHintRegistrationOptions;

      /**
       * The server has support for pull model diagnostics.
       *
       * @since 3.17.0
       */
      // diagnosticProvider ?: DiagnosticOptions | DiagnosticRegistrationOptions;

      /**
       * The server provides workspace symbol support.
       */
      // workspaceSymbolProvider ?: boolean | WorkspaceSymbolOptions;

      /**
       * Workspace specific server capabilities
       */
      // workspace ?:
      // {
      //       /**
      //        * The server supports workspace folder.
      //        *
      //        * @since 3.6.0
      //        */
      //       workspaceFolders ?: WorkspaceFoldersServerCapabilities;

      //       /**
      //        * The server is interested in file notifications/requests.
      //        *
      //        * @since 3.16.0
      //        */
      //       fileOperations ?:
      //       {
      //             /**
      //              * The server is interested in receiving didCreateFiles
      //              * notifications.
      //              */
      //             didCreate ?: FileOperationRegistrationOptions;

      //             /**
      //              * The server is interested in receiving willCreateFiles requests.
      //              */
      //             willCreate ?: FileOperationRegistrationOptions;

      //             /**
      //              * The server is interested in receiving didRenameFiles
      //              * notifications.
      //              */
      //             didRename ?: FileOperationRegistrationOptions;

      //             /**
      //              * The server is interested in receiving willRenameFiles requests.
      //              */
      //             willRename ?: FileOperationRegistrationOptions;

      //             /**
      //              * The server is interested in receiving didDeleteFiles file
      //              * notifications.
      //              */
      //             didDelete ?: FileOperationRegistrationOptions;

      //             /**
      //              * The server is interested in receiving willDeleteFiles file
      //              * requests.
      //              */
      //             willDelete ?: FileOperationRegistrationOptions;
      //       };
      // };

      /**
       * Experimental server capabilities.
       */
      // experimental ?: LSPAny;
};

struct InitializeResult
{
      ServerCapabilities capabilities;
      ServerInfo serverInfo;
};

struct Position
{
      uint line;
      uint character;
};

struct Range
{
      Position start;
      Position end;
};

typedef std::string DocumentUri;
struct Location
{
      DocumentUri uri;
      Range range;
};

struct textDocumentIdentifier {
      std::string uri;
};

struct versionedTextDocumentIdentifier: public textDocumentIdentifier {
      int version;
};

struct textDocument {
      static constexpr const char word_delimiters[] = " `~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?";
      // std::string m_uri;
      std::string m_content;

      textDocument();
      textDocument(const std::string& content);
      std::string getLine(int n);
      static bool isWordDelimiter(const char c);
      std::string wordUnderCursor(Position cursorPosition);
      int findPos(const Position& position) const;
};

struct textDocumentPositionParams
{
      textDocumentIdentifier textDocument;
      Position position;
};

typedef std::string ProgressToken;

struct workDoneProgressParams {
      std::optional<ProgressToken> workDoneToken;
};

struct PartialResultParams {
      std::optional<ProgressToken> partialResultToken;
};

namespace MarkupKind {
      static constexpr const char* PlainText = "plaintext";
      static constexpr const char* Markdown = "markdown";
}

struct MarkupContent
{
      std::string kind;
      std::string value;
};

struct TextDocumentContentChangeEvent {
      std::optional<Range> range;
      std::optional<uint> rangeLength;
      std::string text;
};

struct DidChangeTextDocumentParams {
      textDocumentIdentifier textDocument;
      std::vector<TextDocumentContentChangeEvent> contentChanges;
};

// Results
struct hoverResult
{
      MarkupContent contents;
      std::optional<Range> range;
};
struct declarationResult: public Location {};
struct definitionResult: public Location {};

// Params
struct hoverParams: public textDocumentPositionParams, workDoneProgressParams, PartialResultParams {};
struct declarationParams: public textDocumentPositionParams, workDoneProgressParams, PartialResultParams {};
struct definitionParams: public textDocumentPositionParams, workDoneProgressParams, PartialResultParams {};


// Serialization
void to_json(nlohmann::json &j, const ServerCapabilities::TextDocumentSyncOptions &syncOptions);
void to_json(nlohmann::json &j, const ServerCapabilities &capabilities);
void to_json(nlohmann::json &j, const ServerInfo &serverInfo);
void to_json(nlohmann::json &j, const InitializeResult &initResult);
void to_json(nlohmann::json &j, const Position &p);
void to_json(nlohmann::json &j, const textDocumentIdentifier &p);
void to_json(nlohmann::json &j, const MarkupContent &p);
void to_json(nlohmann::json &j, const hoverResult &h);
void to_json(nlohmann::json &j, const Range &r);
void to_json(nlohmann::json &j, const Location &l);
void to_json(nlohmann::json &j, const textDocumentPositionParams &td);

// Deserialization
void from_json(const nlohmann::json &j, Position &p);
void from_json(const nlohmann::json &j, textDocumentIdentifier &p);
void from_json(const nlohmann::json &j, Range &r);
void from_json(const nlohmann::json &j, Location &l);
void from_json(const nlohmann::json &j, textDocumentPositionParams &td);
void from_json(const nlohmann::json &j, TextDocumentContentChangeEvent &td);
void from_json(const nlohmann::json &j, DidChangeTextDocumentParams &p);
void from_json(const nlohmann::json &j, versionedTextDocumentIdentifier &td);
