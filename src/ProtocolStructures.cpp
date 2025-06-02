#include "ProtocolStructures.hpp"

void to_json(nlohmann::json &j, const ServerCapabilities::TextDocumentSyncOptions &syncOptions){
      j = nlohmann::json{{"change", syncOptions.change}, {"openClose", syncOptions.openClose}};
}

void to_json(nlohmann::json &j, const ServerCapabilities &capabilities)
{
      // TODO: handle encoding
      j = nlohmann::json{{"positionEncoding", "utf-16"}, {"textDocumentSync", capabilities.textDocumentSync}};

      if (capabilities.advertisedCapabilities & ServerCapabilities::completionProvider) j["completionProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::hoverProvider) j["hoverProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::signatureHelpProvider) j["signatureHelpProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::declarationProvider) j["declarationProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::definitionProvider) j["definitionProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::typeDefinitionProvider) j["typeDefinitionProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::implementationProvider) j["implementationProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::referencesProvider) j["referencesProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentHighlightProvider) j["documentHighlightProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentSymbolProvider) j["documentSymbolProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::codeActionProvider) j["codeActionProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::codeLensProvider) j["codeLensProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentLinkProvider) j["documentLinkProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::colorProvider) j["colorProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentFormattingProvider) j["documentFormattingProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentRangeFormattingProvider) j["documentRangeFormattingProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::documentOnTypeFormattingProvider) j["documentOnTypeFormattingProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::renameProvider) j["renameProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::foldingRangeProvider) j["foldingRangeProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::executeCommandProvider) j["executeCommandProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::selectionRangeProvider) j["selectionRangeProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::linkedEditingRangeProvider) j["linkedEditingRangeProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::callHierarchyProvider) j["callHierarchyProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::semanticTokensProvider) j["semanticTokensProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::monikerProvider) j["monikerProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::typeHierarchyProvider) j["typeHierarchyProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::inlineValueProvider) j["inlineValueProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::inlayHintProvider) j["inlayHintProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::diagnosticProvider) j["diagnosticProvider"] = true;
      if (capabilities.advertisedCapabilities & ServerCapabilities::workspaceSymbolProvider) j["workspaceSymbolProvider"] = true;
}

void to_json(nlohmann::json &j, const ServerInfo &serverInfo)
{
      j = nlohmann::json{{"name", serverInfo.name}, {"version", serverInfo.version}};
}

void to_json(nlohmann::json &j, const InitializeResult &initResult)
{
      j = nlohmann::json{{"capabilities", initResult.capabilities}, {"serverInfo", initResult.serverInfo}};
}

void to_json(nlohmann::json &j, const Position &p)
{
      j = {{"line", p.line}, {"character", p.character}};
}

void to_json(nlohmann::json &j, const textDocumentIdentifier &p)
{
      j = {{"uri", p.uri}};
}

void to_json(nlohmann::json &j, const MarkupContent &p)
{
      j = {{"kind", p.kind}, {"value", p.value}};
}
void to_json(nlohmann::json &j, const hoverResult &h)
{
      j = {{"contents", h.contents}};

      if (h.range.has_value()) j["range"] = h.range.value();
}

void to_json(nlohmann::json &j, const Range &r)
{
      j = {{"start", r.start}, {"end", r.end}};
}

void to_json(nlohmann::json &j, const Location &l)
{
      j = {{"range", l.range}, {"uri", l.uri}};
}

void to_json(nlohmann::json &j, const textDocumentPositionParams &td)
{
      j = {{"textDocument", td.textDocument}, {"position", td.position}};
}

void from_json(const nlohmann::json &j, Position &p)
{
      j.at("line").get_to(p.line);
      j.at("character").get_to(p.character);
}

void from_json(const nlohmann::json &j, textDocumentIdentifier &p)
{
      j.at("uri").get_to(p.uri);
}

void from_json(const nlohmann::json &j, Range &r)
{
      j.at("start").get_to(r.start);
      j.at("end").get_to(r.end);
}

void from_json(const nlohmann::json &j, Location &l)
{
      j.at("range").get_to(l.range);
      j.at("uri").get_to(l.uri);
}

void from_json(const nlohmann::json &j, textDocumentPositionParams &td)
{
      j.at("textDocument").get_to(td.textDocument);
      j.at("position").get_to(td.position);
}

void from_json(const nlohmann::json &j, TextDocumentContentChangeEvent &td)
{
      j.at("text").get_to(td.text);
      if (j.contains("range")) j.at("range").get_to(td.range);
      if (j.contains("rangeLength")) j.at("rangeLength").get_to(td.rangeLength);
}

void from_json(const nlohmann::json &j, DidChangeTextDocumentParams &p)
{
      p.contentChanges = j.at("contentChanges");
      j.at("textDocument").get_to(p.textDocument);
}

void from_json(const nlohmann::json &j, versionedTextDocumentIdentifier &td)
{
      j.at("uri").get_to(td.uri);
      j.at("version").get_to(td.version);
}
