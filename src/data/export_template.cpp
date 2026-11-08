//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/export_template.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/field.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <util/io/package_manager.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

// ----------------------------------------------------------------------------- : Export template, basics

ExportTemplate::ExportTemplate()
  : file_type(_("HTML files (*.html)|*.html"))
  , create_directory(false)
{}

ExportTemplateP ExportTemplate::byName(const String& name) {
  return package_manager.open<ExportTemplate>(add_extension(name, _(".mse-export-template")));
}

String ExportTemplate::typeNameStatic() { return _("export-template"); }
String ExportTemplate::typeName() const { return _("export-template"); }
Version ExportTemplate::fileVersion() const { return file_version_export_template; }

void ExportTemplate::validate(Version) {
  if (!game) {
    throw Error(_ERROR_1_("no game specified",_TYPE_("export template")));
  }
  // an export template depends on the game it is made for
  requireDependency(game.get());
}


IMPLEMENT_REFLECTION(ExportTemplate) {
  REFLECT_BASE(Packaged);
  REFLECT(game);
  REFLECT(file_type);
  REFLECT(create_directory);
  REFLECT(option_fields);
  REFLECT_IF_READING option_style.init(option_fields);
  REFLECT(option_style);
  REFLECT(script);
}

// ----------------------------------------------------------------------------- : ExportInfo

IMPLEMENT_DYNAMIC_ARG(ExportInfo*, export_info, nullptr);

ExportInfo::ExportInfo() : allow_writes_outside(false) {}

// ----------------------------------------------------------------------------- : Exporting a set

// Used by the --export command line option.
// MSE Slim: moved here from the removed gui/html_export_window.cpp, unchanged.
ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname) {
  wxBusyCursor wait;
  // export info for script
  ExportInfo info;
  info.export_template = exp;
  info.set = set;
  WITH_DYNAMIC_ARG(export_info, &info);
  // create directory?
  if (exp->create_directory) {
    if (outname.empty()) throw Error(_("No output filename specified for export"));
    wxFileName fn(outname);
    info.directory_relative = fn.GetName() + _("-files");
    fn.SetFullName(info.directory_relative);
    info.directory_absolute = fn.GetFullPath();
    if (!wxDirExists(info.directory_absolute)) {
      wxMkdir(info.directory_absolute);
    }
  }
  // run export script
  Context& ctx = set->getContext();
  LocalScope scope(ctx);
  ctx.setVariable(_("cards"), to_script(&cards));
  ctx.setVariable(_("options"), to_script(&settings.exportOptionsFor(*exp)));
  ctx.setVariable(_("directory"), to_script(info.directory_relative));
  ScriptValueP result = exp->script.invoke(ctx);
  // Save to file
  if (!outname.empty()) {
    // TODO: write as image?
    // write as string
    wxFileOutputStream file(outname);
    wxTextOutputStream stream(file);
    stream.WriteString(result->toString());
  }
  return result;
}
