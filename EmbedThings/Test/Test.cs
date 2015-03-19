using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Embed
{
    public class Test
    {
		public Test()
		{
			Console.WriteLine("Yellow!");
		}

		public static void Recompile()
		{
			var psi = new ProcessStartInfo("mcs", @"-target:library C:\code\stuff\embed\EmbedThings\Test\Test.cs -out:C:\code\stuff\build\Managed\Test.dll");
			psi.WorkingDirectory = @"C:\code\stuff\build\mono\bin";
			var p = new Process();
			p.StartInfo = psi;
			p.Start();
		}
    }
}
