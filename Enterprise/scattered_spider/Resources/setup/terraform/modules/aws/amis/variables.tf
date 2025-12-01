variable "ami_owners" {
  description = "List of AMI owners to filter by"
  type        = list(string)
  default     = ["self"]
}
