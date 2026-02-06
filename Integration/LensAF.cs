using NINA.Plugin.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.Integration {
    public class LensAF {
        public class RegisterFocuser(string focuserName) : IMessage {
            public Guid SenderId => Guid.Parse(NEKMediator.Plugin.Identifier);
            public string Sender => nameof(NEK);
            public DateTimeOffset SentAt => DateTime.UtcNow;
            public Guid MessageId => Guid.NewGuid();
            public DateTimeOffset? Expiration => null;
            public Guid? CorrelationId => null;
            public int Version => 1;
            public IDictionary<string, object> CustomHeaders => new Dictionary<string, object>();
            public string Topic => "LensAF.RegisterFocuser";
            public object Content => focuserName;


            public async static void Send(string focuserName) {
                await NEKMediator.MessageBroker.Publish(new RegisterFocuser(focuserName));
            }
        }


        public class GotoFocus() : IMessage {
            public Guid SenderId => Guid.Parse(NEKMediator.Plugin.Identifier);
            public string Sender => nameof(NEK);
            public DateTimeOffset SentAt => DateTime.UtcNow;
            public Guid MessageId => Guid.NewGuid();
            public DateTimeOffset? Expiration => null;
            public Guid? CorrelationId => null;
            public int Version => 1;
            public IDictionary<string, object> CustomHeaders => new Dictionary<string, object>();
            public string Topic => "LensAF.GotoFocus";
            public object Content => new();


            public async static void Send() {
                await NEKMediator.MessageBroker.Publish(new GotoFocus());
            }
        }
    }
}
