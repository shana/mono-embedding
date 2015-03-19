using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Threading;


namespace EmbedThings
{
	class EntryPoint
	{
		bool running = false;
		ManualResetEvent evt = new ManualResetEvent(false);

		static void Main(string[] args)
		{
			new EntryPoint().Run();
		}

		public int Run()
		{
			bool running = true;

			Console.WriteLine("Hello!");
			while (running) {

				Console.Write("> ");

				string line = "";

				line = Console.ReadLine();

				if (line.Equals("r")) {
					reload();
					break;
				} else if (line.Equals("q")) {
					Console.WriteLine("Bye bye!");
					return 0;
				}
			}
			return 2;
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		extern static void reload();

		public void OnReload()
		{
			running = false;
		}
	}
}
