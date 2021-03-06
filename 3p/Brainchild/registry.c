/*
 *	registry.c
 *
 *	(C) Copyright 1999-2005 Jan van den Baard.
 *	    All Rights Reserved.
 */

#include "defs.h"

/*
 *	The registery key.
 */
static LPCTSTR	lpszProgramKey    = _T("Software\\Stephan Brenner\\Brainchild"),   // Modified by Stephan (2005-05-28)
		lpszSearchStrKey  = _T("Search"),
		lpszReplaceStrKey = _T("Replace"),
		lpszPrefDirKey    = _T("Settings");

/*
 *	Storage for the settings path.
 */
TCHAR		szSettingsPath[ MAX_PATH ];

/*
 *	Check for the presence of the
 *	backslash at the end of the
 *	path name.
 */
static void CheckBackslash( LPTSTR lpszPath )
{
	if ( lpszPath[ _tcslen( lpszPath ) - 1 ] != _T( '\\' ))
		_tcscat( lpszPath, _T("\\"));
}

/*
 *	Build user path from the settings
 *	path.
 *
 *	Does not check buffer length.
 */
static void BuildUserPath( LPTSTR lpszUserPath )
{
	DWORD		dwUserSize = MAX_PATH;

  // Modified by Stephan (2005-05-28)
	_stprintf(lpszUserPath, _T("%s\\Defaults"), szSettingsPath);
}

/*
 *	Copy default settings files. This code simply
 *	copies _all_ files which are located in the
 *	"Defaults" directory in the path specified by
 *	the key:
 *
 *	HKEY_LOCAL_MACHINE\BCDLL\Currentversion\Settings
 */
static BOOL CopyDefaultFiles( LPCTSTR lpszUserPath )
{
	HANDLE		hFile;
	WIN32_FIND_DATA	wfd;
	TCHAR		szCurrDir[ MAX_PATH ];
	TCHAR		szDefDir[ MAX_PATH ];
	TCHAR		szDestName[ MAX_PATH ];
	BOOL		bRC = TRUE;

	/*
	 *	Save current directory.
	 */
	if ( GetCurrentDirectory( MAX_PATH, szCurrDir ) > 0 )
	{
		/*
		 *	Copy settings path.
		 */
		_tcscpy( szDefDir, szSettingsPath );

		/*
		 *	Append a "\" to the path name.
		 */
		CheckBackslash( szDefDir );

		/*
		 *	Append the location of the default
		 *	files.
		 */
		_tcscat( szDefDir, _T("Defaults"));

		/*
		 *	Go to the default directory.
		 */
		if ( SetCurrentDirectory( szDefDir ))
		{
			/*
			 *	Copy all files...
			 */
			if (( hFile = FindFirstFile( _T("*.*"), &wfd )) != INVALID_HANDLE_VALUE )
			{
				/*
				 *	Get and copy all files.
				 */
				do
				{
					/*
					 *	Build target name...
					 */
					BuildUserPath( szDestName );

					/*
					 *	Append a "\" is necessary.
					 */
					CheckBackslash( szDestName );

					/*
					 *	Append file name.
					 */
					_tcscat( szDestName, wfd.cFileName );

					/*
					 *	Copy the file... No error checking.
					 */
					CopyFile( wfd.cFileName, szDestName, FALSE );
				} while ( FindNextFile( hFile, &wfd ));

				/*
				 *	Close the handle.
				 */
				FindClose( hFile );
			}
			else
				bRC = FALSE;

			/*
			 *	Get back to the original directory.
			 */
			SetCurrentDirectory( szCurrDir );
		}
		else
		{
		//	MessageBox( NULL, szDefDir, NULL, MB_OK );
			bRC = FALSE;
		}
	}
	else
		bRC = FALSE;

	return bRC;
}

/*
 *	Check is the current user already
 *	has a settings entry and, if not,
 *	create a directory for the user and
 *	copy the default files to it.
 */
BOOL CheckUser( LPTSTR lpszUserPath )
{
	TCHAR		szCurrDir[ MAX_PATH ];
	TCHAR		szUserPath[ MAX_PATH ];
	BOOL		bRC = TRUE;

	/*
	 *	Save current directory.
	 */
	if ( GetCurrentDirectory( MAX_PATH, szCurrDir ) > 0 )
	{
		/*
		 *	Get the current user path.
		 */
		BuildUserPath( szUserPath );

		/*
		 *	Go to the user path.
		 */
		if ( SetCurrentDirectory( szUserPath ) == FALSE )
		{
			/*
			 *	We assume the path does not exist.
			 */

			/*
			 *	Create the directory.
			 */
			if ( CreateDirectory( szUserPath, NULL ))
				/*
				 *	Copy default files...
				 */
				bRC = CopyDefaultFiles( szUserPath );
			else
				bRC = FALSE;
		}

		/*
		 *	Go back to the original directory.
		 */
		SetCurrentDirectory( szCurrDir );
	} 
	else
		bRC = FALSE;

	/*
	 *	Copy the path on success.
	 */
	if ( bRC )
		_tcscpy( lpszUserPath, szUserPath );

	return bRC;
}

BOOL GetSettingsLocation( void )
{
  // Modified by Stephan Brenner (2005-01-06)

  TCHAR szTempPath[MAX_PATH];
  GetTempPath(MAX_PATH, szTempPath);

  _stprintf(szSettingsPath, _T("%s\\Brainchild"), szTempPath);

	return TRUE;
}

BOOL SaveSearchReplaceStrings( LPCLASSDATA lpcd, LPSTRINGLIST lpList, BOOL bReplace )
{
	HKEY		hKey;
	DWORD		dwSize;
	BOOL		bRC = FALSE;
	TCHAR		szKey[ 16 ];
	LPSTRING	pStr;
	int		i;

	/*
	 *	Open/create the key.
	 */
	if ( RegCreateKeyEx( HKEY_CURRENT_USER, lpszProgramKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL ) == ERROR_SUCCESS )
	{
		/*
		 *	First we delete the current registry contents.
		 */
		for ( i = 0; i < 30; i++ )
		{
			/*
			 *	Format value name.
			 */
			_stprintf( szKey, _T( "%s%ld" ), bReplace ? lpszSearchStrKey : lpszReplaceStrKey, i );
			if ( RegDeleteValue( hKey, szKey ) == ERROR_FILE_NOT_FOUND )
				break;
		}

		/*
		 *	Now we write the list one-by-one.
		 */
		for ( pStr = lpList->lpFirst, i = 0; pStr->lpNext ; pStr = pStr->lpNext, i++ )
		{
			/*
			 *	Format value name.
			 */
			_stprintf( szKey, _T( "%s%ld" ), bReplace == FALSE ? lpszSearchStrKey : lpszReplaceStrKey, i );

			/*
			 *	Setup length.
			 */
			dwSize = _tcslen( pStr->lpszString ) + 1;

			/*
			 *	Write the string.
			 */
			if ( RegSetValueEx( hKey, szKey, 0, REG_SZ, ( LPBYTE )pStr->lpszString, dwSize ) == ERROR_SUCCESS )
				bRC = TRUE;
		}

		/*
		 *	Close the key.
		 */
		RegCloseKey( hKey );
	}
	return bRC;
}

BOOL LoadSearchReplaceStrings( LPCLASSDATA lpcd, LPSTRINGLIST lpList, BOOL bReplace )
{
	HKEY		hKey;
	DWORD		dwSize = sizeof( DWORD );
	LPTSTR		pszString = NULL;
	TCHAR		szKey[ 16 ];
	BOOL		bRC = FALSE;
	int		i;

	/*
	 *	Delete the current contents of the list.
	 */
	FreeStringNodes( lpcd, lpList );

	/*
	 *	Open the key.
	 */
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, lpszProgramKey, 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		/*
		 *	Get upto nMaxTrack strings.
		 */
		for ( i = 0; i < lpcd->nMaxTrack; i++ )
		{
			/*
			 *	Format value name.
			 */
			_stprintf( szKey, _T( "%s%ld" ), bReplace == FALSE ? lpszSearchStrKey : lpszReplaceStrKey, i );

			/*
			 *	Get the length required.
			 */
			if ( RegQueryValueEx( hKey, szKey, 0, NULL, NULL, &dwSize ) != ERROR_SUCCESS )
				break;

			/*
			 *	Preset buffer length.
			 */
			dwSize = REAL_SIZE( dwSize );

			/*
			 *	Allocate memory for the string.
			 */
			if (( pszString = AllocPooled( pMainPool, dwSize )) != NULL )
			{
				/*
				 *	Read the string.
				 */
				if ( RegQueryValueEx( hKey, szKey, 0, NULL, ( LPBYTE )pszString, &dwSize ) == ERROR_SUCCESS )
				{
					/*
					 *	Add it to the list.
					 */
					if ( AddStringNode( lpcd, lpList, pszString, TRUE ))
						bRC = TRUE;
				}

				/*
				 *	Free the string.
				 */
				FreePooled( pMainPool, pszString );
			}
		}

		/*
		 *	Close the key.
		 */
		RegCloseKey( hKey );
	}
	return bRC;
}
