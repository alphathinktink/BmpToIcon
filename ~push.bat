@echo off

echo.
echo Adding files...
git add -A || (
	echo.
	echo ERROR: git add failed.
	pause
	exit /b 1
)

echo.
echo Files staged for commit:
git diff --cached --name-status

echo.
echo Committing changes...
git commit -v -m "ll" || (
	echo.
	echo ERROR: git commit failed.
	echo This usually means nothing changed or there is a commit issue.
	pause
	exit /b 1
)

echo.
echo Pushing to GitHub...
git push || (
	echo.
	echo ERROR: git push failed.
	pause
	exit /b 1
)

echo.
echo Git update completed successfully.
pause
