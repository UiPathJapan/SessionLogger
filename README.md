# SessionLogger

ユーザーセッションのログオンやログオフをログするWindowsサービスです。

既定で次のファイルに記録を書き込みます。

%ProgramData%\UiPathTeam\SessionLogger\Session.log

### 設定

レジストリの設定により書き込み先を変更できます。

キー HKEY_LOCAL_MACHINE\SOFTWARE\UiPathTeam\SessionLogger 
データ LogFileName

### 使用法

管理者モードで起動したコマンドプロンプトから

サービスをインストール:

```
SessionLogger.exe install
```

サービスをアンインストール:

```
SessionLogger.exe uninstall
```

サービスを起動:

```
net start UiPathTeamSessionLogger
```

サービスを停止:

```
net stop UiPathTeamSessionLogger
```
