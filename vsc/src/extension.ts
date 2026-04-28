import * as vscode from "vscode";
import { spawn } from "child_process";

export function activate(context: vscode.ExtensionContext) {
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
