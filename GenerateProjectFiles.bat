cd %~d0%~p0
setlocal
powershell -command "$uproject = Get-ChildItem "*.uproject" -Name; $bin = & { (Get-ItemProperty 'Registry::HKEY_CLASSES_ROOT\Unreal.ProjectFile\shell\rungenproj' -Name 'Icon' ).'Icon' }; $bin + ' -projectfiles %cd%\' + $uproject" | cmd.exe