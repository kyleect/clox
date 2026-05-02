import * as vscode from "vscode";
import { readFileSync } from "node:fs";
import { join } from "node:path";

export function activate(context: vscode.ExtensionContext) {
  const EXT_ROOT = context.extensionPath;

  vscode.languages.registerHoverProvider("clox", {
    provideHover(doc, position) {
      const currentWord = doc.getText(doc.getWordRangeAtPosition(position));

      let contents: string = "";

      switch (currentWord) {
        case "this":
          contents = readFileSync(join(EXT_ROOT, "hovers", "this.md"), "utf8");
          break;
        case "exit":
          contents = readFileSync(join(EXT_ROOT, "hovers", "exit.md"), "utf8");
          break;
        case "argc":
          contents = readFileSync(join(EXT_ROOT, "hovers", "argc.md"), "utf8");
          break;
        case "argv":
          contents = readFileSync(join(EXT_ROOT, "hovers", "argv.md"), "utf8");
          break;
        case "len":
          contents = readFileSync(join(EXT_ROOT, "hovers", "len.md"), "utf8");
          break;
        case "__version__":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "__version__.md"),
            "utf8",
          );
          break;
        case "setenv":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "setenv.md"),
            "utf8",
          );
          break;
        case "getenv":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "getenv.md"),
            "utf8",
          );
          break;
        case "typeof":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "typeof.md"),
            "utf8",
          );
          break;
        case "instanceOf":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "instanceOf.md"),
            "utf8",
          );
          break;
        case "stdin":
          contents = readFileSync(join(EXT_ROOT, "hovers", "stdin.md"), "utf8");
          break;
        case "prompt":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "prompt.md"),
            "utf8",
          );
          break;
        case "readFileToString":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "readFileToString.md"),
            "utf8",
          );
          break;
        case "fileExists":
          contents = readFileSync(
            join(EXT_ROOT, "hovers", "fileExists.md"),
            "utf8",
          );
          break;
        case "ceil":
          contents = readFileSync(join(EXT_ROOT, "hovers", "ceil.md"), "utf8");
          break;
        case "clock":
          contents = readFileSync(join(EXT_ROOT, "hovers", "clock.md"), "utf8");
          break;
      }

      return new vscode.Hover(new vscode.MarkdownString(contents));
    },
  });

  const runCommand = vscode.commands.registerCommand("clox.run", () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) return;

    const file = editor.document.fileName;

    const terminal = vscode.window.createTerminal("Clox");
    terminal.show();

    terminal.sendText(`./build/clox -f "${file}"`);
  });

  context.subscriptions.push(runCommand);
}
